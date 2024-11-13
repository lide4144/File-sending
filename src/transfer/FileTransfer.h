#pragma once
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QFile>

class FileTransfer : public QObject {
    Q_OBJECT
public:
    explicit FileTransfer(QObject *parent = nullptr);
    ~FileTransfer();
    
    // 连接到服务器
    bool connectToServer(const QString &address, quint16 port = 8080);
    // 发送文件
    bool sendFile(const QString &filePath);
    // 取消传输
    void cancelTransfer();
    // 断开连接
    void disconnect();
    // 获取连接状态
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void transferProgress(qint64 bytesSent, qint64 totalBytes);
    void transferCompleted();
    void transferError(const QString &error);

private slots:
    void handleConnected();
    void handleDisconnected();
    void handleBytesWritten(qint64 bytes);
    void handleError(QAbstractSocket::SocketError socketError);

private:
    void resetTransfer();
    bool sendFileHeader();
    bool sendFileData();

    QTcpSocket *socket;
    QFile *currentFile;
    qint64 totalBytes;
    qint64 bytesSent;
    bool transferring;
}; 