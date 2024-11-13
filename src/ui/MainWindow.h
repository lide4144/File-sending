#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include "../server/FileServer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void handleClientConnected(const QString &clientAddress);
    void handleFileReceiveStarted(const QString &fileName, qint64 fileSize);
    void handleFileReceiveProgress(qint64 bytesReceived);
    void handleFileReceiveCompleted();
    void handleStartServer();
    void handleStopServer();

private:
    void setupUi();
    FileServer *fileServer;
    
    // UI组件
    QLabel *statusLabel;
    QLabel *connectionLabel;
    QProgressBar *progressBar;
    QPushButton *startServerButton;
    QPushButton *stopServerButton;
    QLabel *ipAddressLabel;
}; 