#include "FileServer.h"
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QNetworkInterface>

FileServer::FileServer(QObject *parent) 
    : QObject(parent)
    , server(new QTcpServer(this))
    , clientSocket(nullptr)
    , currentFile(nullptr)
    , transferState(TransferState::WaitingHeader)
    , fileSize(0)
    , receivedSize(0) {
    
    // 设置默认保存目录为下载文件夹
    saveDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir().mkpath(saveDirectory);
}

FileServer::~FileServer() {
    stopServer();
    delete currentFile;
}

bool FileServer::startServer(quint16 port) {
    if (server->isListening()) {
        return true;
    }
    
    return server->listen(QHostAddress::Any, port);
}

void FileServer::stopServer() {
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }
    
    if (server->isListening()) {
        server->close();
    }
    
    resetTransferState();
}

QString FileServer::getServerAddress() const {
    QString address;
    for(const QHostAddress &addr: QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) {
            address = addr.toString();
            break;
        }
    }
    return address;
}

void FileServer::handleNewConnection() {
    if (clientSocket) {
        // 如果已有连接，拒绝新连接
        server->nextPendingConnection()->deleteLater();
        return;
    }
    
    clientSocket = server->nextPendingConnection();
    
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &FileServer::handleReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &FileServer::handleDisconnected);
    connect(clientSocket, &QTcpSocket::errorOccurred,
            this, &FileServer::handleError);
            
    emit clientConnected(clientSocket->peerAddress().toString());
    resetTransferState();
}

void FileServer::handleReadyRead() {
    buffer.append(clientSocket->readAll());
    
    switch (transferState) {
        case TransferState::WaitingHeader:
            processFileHeader();
            break;
            
        case TransferState::ReceivingFile:
            processFileData();
            break;
    }
}

void FileServer::processFileHeader() {
    // 检查是否收到完整的文件头信息
    if (buffer.size() < sizeof(qint64) + sizeof(qint32)) {
        return;
    }
    
    QDataStream stream(buffer);
    stream >> fileSize;
    
    qint32 fileNameSize;
    stream >> fileNameSize;
    
    if (buffer.size() < sizeof(qint64) + sizeof(qint32) + fileNameSize) {
        return;
    }
    
    QByteArray fileNameData = buffer.mid(sizeof(qint64) + sizeof(qint32), fileNameSize);
    currentFileName = QString::fromUtf8(fileNameData);
    
    // 准备文件保存
    QString savePath = getSaveFilePath(currentFileName);
    currentFile = new QFile(savePath);
    
    if (!currentFile->open(QIODevice::WriteOnly)) {
        emit error(tr("无法创建文件: %1").arg(savePath));
        resetTransferState();
        return;
    }
    
    // 更新状态
    buffer = buffer.mid(sizeof(qint64) + sizeof(qint32) + fileNameSize);
    transferState = TransferState::ReceivingFile;
    emit fileReceiveStarted(currentFileName, fileSize);
    
    // 如果buffer中还有数据，继续处理
    if (!buffer.isEmpty()) {
        processFileData();
    }
}

void FileServer::processFileData() {
    if (!currentFile || !currentFile->isOpen()) {
        resetTransferState();
        return;
    }
    
    // 写入数据
    qint64 written = currentFile->write(buffer);
    if (written == -1) {
        emit error(tr("写入文件失败: %1").arg(currentFile->errorString()));
        resetTransferState();
        return;
    }
    
    receivedSize += written;
    buffer.clear();
    
    emit fileReceiveProgress(receivedSize);
    
    // 检查是否接收完成
    if (receivedSize >= fileSize) {
        currentFile->close();
        emit fileReceiveCompleted();
        resetTransferState();
    }
}

void FileServer::handleDisconnected() {
    emit clientDisconnected();
    clientSocket->deleteLater();
    clientSocket = nullptr;
    resetTransferState();
}

void FileServer::handleError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit error(clientSocket->errorString());
}

void FileServer::resetTransferState() {
    if (currentFile) {
        currentFile->close();
        delete currentFile;
        currentFile = nullptr;
    }
    
    transferState = TransferState::WaitingHeader;
    fileSize = 0;
    receivedSize = 0;
    currentFileName.clear();
    buffer.clear();
}

QString FileServer::getSaveFilePath(const QString &fileName) {
    QString baseName = QFileInfo(fileName).baseName();
    QString extension = QFileInfo(fileName).suffix();
    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString newFileName = QString("%1_%2.%3").arg(baseName, dateTime, extension);
    return QDir(saveDirectory).filePath(newFileName);
} 