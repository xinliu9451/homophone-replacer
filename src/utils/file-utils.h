// file-utils.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <string>
#include <vector>

namespace hr_standalone {

// 检查文件是否存在
bool FileExists(const std::string &filename);

// 如果文件不存在则退出程序
void AssertFileExists(const std::string &filename);

// 读取文件内容到vector
std::vector<char> ReadFile(const std::string &filename);

}  // namespace hr_standalone

#endif  // FILE_UTILS_H_
