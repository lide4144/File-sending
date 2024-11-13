#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <mutex>

class FileServer {
public:
    FileServer(const std::string& root_path);
    
    // 文件操作
    bool uploadFile(const std::string& filename, const std::vector<char>& data);
    std::vector<char> downloadFile(const std::string& filename);
    bool deleteFile(const std::string& filename);
    std::vector<std::string> listFiles() const;
    
    // 用户认证
    bool authenticate(const std::string& username, const std::string& password);
    bool addUser(const std::string& username, const std::string& password);
    
private:
    std::filesystem::path root_path_;
    std::unordered_map<std::string, std::string> users_; // username -> password
    mutable std::mutex mutex_;
    
    void logOperation(const std::string& operation, const std::string& filename);
    
    enum class ErrorCode {
        SUCCESS,
        FILE_NOT_FOUND,
        PERMISSION_DENIED,
        ALREADY_EXISTS,
        UNKNOWN_ERROR
    };
    
    struct Result {
        ErrorCode code;
        std::string message;
    };
}; 