#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QNetworkInterface>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    fileServer = new FileServer(this);
    
    connect(fileServer, &FileServer::clientConnected,
            this, &MainWindow::handleClientConnected);
    connect(fileServer, &FileServer::fileReceiveStarted,
            this, &MainWindow::handleFileReceiveStarted);
    connect(fileServer, &FileServer::fileReceiveProgress,
            this, &MainWindow::handleFileReceiveProgress);
    connect(fileServer, &FileServer::fileReceiveCompleted,
            this, &MainWindow::handleFileReceiveCompleted);
}

void MainWindow::setupUi() {
    // 创建中心部件和主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);
    
    // 创建服务器控制部分
    QHBoxLayout *serverControlLayout = new QHBoxLayout();
    startServerButton = new QPushButton("启动服务器", this);
    stopServerButton = new QPushButton("停止服务器", this);
    stopServerButton->setEnabled(false);
    
    serverControlLayout->addWidget(startServerButton);
    serverControlLayout->addWidget(stopServerButton);
    
    // 创建状态显示部分
    statusLabel = new QLabel("服务器未启动", this);
    connectionLabel = new QLabel("等待连接...", this);
    ipAddressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    
    // 获取本机IP地址
    QString ipAddresses = "本机IP地址:\n";
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for(const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            ipAddresses += address.toString() + "\n";
        }
    }
    ipAddressLabel->setText(ipAddresses);
    
    // 添加所有组件到主布局
    mainLayout->addLayout(serverControlLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(connectionLabel);
    mainLayout->addWidget(ipAddressLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addStretch();
    
    // 设置窗口属性
    setWindowTitle("WiFi文件传输");
    resize(400, 300);
    
    // 连接信号和槽
    connect(startServerButton, &QPushButton::clicked, this, &MainWindow::handleStartServer);
    connect(stopServerButton, &QPushButton::clicked, this, &MainWindow::handleStopServer);
}

void MainWindow::handleStartServer() {
    if (fileServer->startServer()) {
        statusLabel->setText("服务器已启动 - 端口: 8080");
        startServerButton->setEnabled(false);
        stopServerButton->setEnabled(true);
    } else {
        QMessageBox::critical(this, "错误", "无法启动服务器");
    }
}

void MainWindow::handleStopServer() {
    fileServer->stopServer();
    statusLabel->setText("服务器已停止");
    startServerButton->setEnabled(true);
    stopServerButton->setEnabled(false);
    connectionLabel->setText("等待连接...");
    progressBar->setVisible(false);
}

void MainWindow::handleClientConnected(const QString &clientAddress) {
    connectionLabel->setText("已连接客户端: " + clientAddress);
}

void MainWindow::handleFileReceiveStarted(const QString &fileName, qint64 fileSize) {
    statusLabel->setText("正在接收文件: " + fileName);
    progressBar->setVisible(true);
    progressBar->setMaximum(fileSize);
    progressBar->setValue(0);
}

void MainWindow::handleFileReceiveProgress(qint64 bytesReceived) {
    progressBar->setValue(bytesReceived);
}

void MainWindow::handleFileReceiveCompleted() {
    statusLabel->setText("文件接收完成");
    progressBar->setVisible(false);
    QMessageBox::information(this, "完成", "文件传输完成！");
} 