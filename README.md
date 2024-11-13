# File Sharing Server

一个基于C++20的文件共享服务器实现。

## 功能特性

- 文件上传/下载
- 用户认证
- 文件删除
- 文件列表
- 操作日志
- 线程安全

## 构建要求

- CMake 3.15+
- LLVM/Clang
- C++20 支持
- Make

## 构建步骤 

bash
mkdir build
cd build
cmake ..
cmake --build .

## 使用方法

cpp

// 创建服务器实例
FileServer server("./storage");

// 添加用户
server.addUser("admin", "password");

// 上传文件
std::vector<char> data = {'H', 'e', 'l', 'l', 'o'};
server.uploadFile("test.txt", data);

// 下载文件
auto content = server.downloadFile("test.txt");

## 许可证

MIT License