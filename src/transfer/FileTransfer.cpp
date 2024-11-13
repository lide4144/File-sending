#include "FileTransfer.h"
#include <QFileInfo>
#include <QDataStream>

FileTransfer::FileTransfer(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , currentFile(nullptr)
    , totalBytes(0)
    , bytesSent(0)
    , transferring(false) {
    
    connect(socket, &QTcpSocket::connected,
            this, &FileTransfer::handleConnected);
    connect(socket, &QTcpSocket::disconnected,
            this, &FileTransfer::handleDisconnected);
    connect(socket, &QTcpSocket::bytesWritten,
            this, &FileTransfer::handleBytesWritten);
    connect(socket, &QTcpSocket::errorOccurred,
            this, &FileTransfer::handleError);
}

FileTransfer::~FileTransfer() {
    disconnect();
    delete currentFile;
}

bool FileTransfer::connectToServer(const QString &address, quint16 port) {
    if (socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }
    
    socket->connectToHost(address, port);
    return socket->waitForConnected(5000); // 5秒超时
}

void FileTransfer::disconnect() {
    if (transferring) {
        cancelTransfer();
    }
    
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

bool FileTransfer::isConnected() const {
    return socket->state() == QAbstractSocket::ConnectedState;
}

bool FileTransfer::sendFile(const QString &filePath) {
    if (!isConnected()) {
        emit transferError("未连接到服务器");
        return false;
    }
    
    if (transferring) {
        emit transferError("当前正在传输文件");
        return false;
    }
    
    currentFile = new QFile(filePath);
    if (!currentFile->open(QIODevice::ReadOnly)) {
        emit transferError("无法打开文件: " + filePath);
        delete currentFile;
        currentFile = nullptr;
        return false;
    }
    
    totalBytes = currentFile->size();
    bytesSent = 0;
    transferring = true;
    
    // 发送文件头信息
    if (!sendFileHeader()) {
        resetTransfer();
        return false;
    }
    
    // 开始发送文件数据
    return sendFileData();
}

void FileTransfer::cancelTransfer() {
    if (transferring) {
        resetTransfer();
        emit transferError("传输已取消");
    }
}

void FileTransfer::handleConnected() {
    emit connected();
}

void FileTransfer::handleDisconnected() {
    resetTransfer();
    emit disconnected();
}

void FileTransfer::handleBytesWritten(qint64 bytes) {
    if (!transferring) return;
    
    bytesSent += bytes;
    emit transferProgress(bytesSent, totalBytes);
    
    if (bytesSent < totalBytes) {
        // 继续发送数据
        sendFileData();
    } else if (bytesSent >= totalBytes) {
        // 传输完成
        emit transferCompleted();
        resetTransfer();
    }
}

void FileTransfer::handleError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit transferError(socket->errorString());
    resetTransfer();
}

bool FileTransfer::sendFileHeader() {
    QFileInfo fileInfo(*currentFile);
    QString fileName = fileInfo.fileName();
    QByteArray fileNameData = fileName.toUtf8();
    
    // 构造文件头信息
    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream << totalBytes;  // 文件大小(8字节)
    stream << (qint32)fileNameData.size();  // 文件名长度(4字节)
    header.append(fileNameData);  // 文件名
    
    // 发送头信息
    qint64 written = socket->write(header);
    return written == header.size();
}

bool FileTransfer::sendFileData() {
    static const qint64 blockSize = 64 * 1024; // 64KB块大小
    
    if (!currentFile || !transferring) {
        return false;
    }
    
    // 读取并发送数据块
    QByteArray block = currentFile->read(blockSize);
    if (!block.isEmpty()) {
        qint64 written = socket->write(block);
        return written == block.size();
    }
    
    return true;
}

void FileTransfer::resetTransfer() {
    if (currentFile) {
        currentFile->close();
        delete currentFile;
        currentFile = nullptr;
    }
    
    transferring = false;
    totalBytes = 0;
    bytesSent = 0;
} 