cmake_minimum_required(VERSION 3.15)

# 设置LLVM编译器
set(CMAKE_C_COMPILER "E:/LLVM/bin/clang.exe")
set(CMAKE_CXX_COMPILER "E:/LLVM/bin/clang++.exe")
set(CMAKE_MAKE_PROGRAM "E:/w64devkit/bin/mingw32-make.exe")

project(FileServer VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加源文件
add_executable(FileServer 
    src/main.cpp
    src/FileServer.cpp
)

# 包含头文件目录
target_include_directories(FileServer PRIVATE src)

# Clang特定设置
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # 启用警告
    add_compile_options(-Wall -Wextra)
    # 使用UTF-8编码
    add_compile_options(-fexec-charset=UTF-8)
endif() 