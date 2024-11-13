#include "FileServer.h"
#include <iostream>
#include <locale>
#include <codecvt>

#ifdef _WIN32
    #include <windows.h>
#endif

int main() {
    // 设置本地化
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);  // Windows下设置控制台UTF-8输出
    #endif
    
    try {
        // 创建文件服务器实例，设置根目录
        FileServer server("./file_storage");
        
        // 添加测试用户
        server.addUser("admin", "password123");
        
        // 测试用户认证
        if (server.authenticate("admin", "password123")) {
            std::cout << "用户认证成功！" << std::endl;
        }
        
        // 测试文件上传
        std::vector<char> test_data = {'H', 'e', 'l', 'l', 'o'};
        if (server.uploadFile("test.txt", test_data)) {
            std::cout << "文件上传成功！" << std::endl;
        }
        
        // 列出所有文件
        auto files = server.listFiles();
        std::cout << "当前文件列表：" << std::endl;
        for (const auto& file : files) {
            std::cout << "- " << file << std::endl;
        }
        
        // 测试文件下载
        auto downloaded_data = server.downloadFile("test.txt");
        if (!downloaded_data.empty()) {
            std::cout << "文件下载成功！内容：";
            for (char c : downloaded_data) {
                std::cout << c;
            }
            std::cout << std::endl;
        }
        
        // 测试文件删除
        if (server.deleteFile("test.txt")) {
            std::cout << "文件删除成功！" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 