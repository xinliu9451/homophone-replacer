// text-utils.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef TEXT_UTILS_H_
#define TEXT_UTILS_H_

#include <string>
#include <vector>

namespace hr_standalone {

// 分割字符串
void SplitStringToVector(const std::string &full, const char *delim,
                         bool omit_empty_strings,
                         std::vector<std::string> *out);

// 分割UTF-8字符串
std::vector<std::string> SplitUtf8(const std::string &text);

// 转换为小写
std::string ToLowerCase(const std::string &s);
void ToLowerCase(std::string *in_out);

// 移除无效的UTF-8序列
std::string RemoveInvalidUtf8Sequences(const std::string &text);

}  // namespace hr_standalone

#endif  // TEXT_UTILS_H_
