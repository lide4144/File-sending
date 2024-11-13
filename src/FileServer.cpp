#include "FileServer.h"
#include <chrono>
#include <sstream>
#include <iostream>
#include <ctime>

FileServer::FileServer(const std::string& root_path) : root_path_(root_path) {
    if (!std::filesystem::exists(root_path_)) {
        std::filesystem::create_directories(root_path_);
    }
}

bool FileServer::uploadFile(const std::string& filename, const std::vector<char>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        auto file_path = root_path_ / filename;
        std::ofstream file(file_path, std::ios::binary);
        if (!file) return false;
        
        file.write(data.data(), data.size());
        logOperation("UPLOAD", filename);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Upload error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<char> FileServer::downloadFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        auto file_path = root_path_ / filename;
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file) return {};
        
        auto size = file.tellg();
        std::vector<char> buffer(size);
        
        file.seekg(0);
        file.read(buffer.data(), size);
        
        logOperation("DOWNLOAD", filename);
        return buffer;
    } catch (const std::exception& e) {
        std::cerr << "Download error: " << e.what() << std::endl;
        return {};
    }
}

void FileServer::logOperation(const std::string& operation, const std::string& filename) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    char time_str[26];
    #ifdef _WIN32
        ctime_s(time_str, sizeof(time_str), &time);
    #else
        ctime_r(&time, time_str);
    #endif
    
    std::string time_string(time_str);
    if (!time_string.empty() && time_string.back() == '\n') {
        time_string.pop_back();
    }
    
    std::ofstream log_file(root_path_ / "server.log", std::ios::app);
    log_file << time_string << " - " << operation << ": " << filename << std::endl;
}

bool FileServer::deleteFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        auto file_path = root_path_ / filename;
        if (std::filesystem::exists(file_path)) {
            std::filesystem::remove(file_path);
            logOperation("DELETE", filename);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Delete error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> FileServer::listFiles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(root_path_)) {
            if (entry.is_regular_file() && entry.path().filename() != "server.log") {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "List files error: " << e.what() << std::endl;
    }
    return files;
}

bool FileServer::authenticate(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_.find(username);
    if (it != users_.end()) {
        return it->second == password;
    }
    return false;
}

bool FileServer::addUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (users_.find(username) == users_.end()) {
        users_[username] = password;
        logOperation("ADD_USER", username);
        return true;
    }
    return false;
} 