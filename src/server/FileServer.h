#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QFile>

class FileServer : public QObject {
    Q_OBJECT
public:
    explicit FileServer(QObject *parent = nullptr);
    ~FileServer();
    bool startServer(quint16 port = 8080);
    void stopServer();
    QString getServerAddress() const;

signals:
    void clientConnected(const QString &clientAddress);
    void clientDisconnected();
    void fileReceiveStarted(const QString &fileName, qint64 fileSize);
    void fileReceiveProgress(qint64 bytesReceived);
    void fileReceiveCompleted();
    void error(const QString &errorMessage);

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();
    void handleError(QAbstractSocket::SocketError socketError);

private:
    void resetTransferState();
    void processFileHeader();
    void processFileData();
    QString getSaveFilePath(const QString &fileName);

    QTcpServer *server;
    QTcpSocket *clientSocket;
    QFile *currentFile;
    
    // 传输状态
    enum class TransferState {
        WaitingHeader,
        ReceivingHeader,
        ReceivingFile
    };
    
    TransferState transferState;
    qint64 fileSize;
    qint64 receivedSize;
    QString currentFileName;
    QByteArray buffer;
    QString saveDirectory;
}; 