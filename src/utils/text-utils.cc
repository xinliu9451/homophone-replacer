// text-utils.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "text-utils.h"
#include <algorithm>
#include <sstream>

namespace hr_standalone {

void SplitStringToVector(const std::string &full, const char *delim,
                         bool omit_empty_strings,
                         std::vector<std::string> *out) {
  out->clear();
  if (full.empty()) return;
  
  size_t start = 0;
  size_t found = 0;
  while ((found = full.find_first_of(delim, start)) != std::string::npos) {
    if (found != start || !omit_empty_strings) {
      out->push_back(full.substr(start, found - start));
    }
    start = found + 1;
  }
  if (start < full.length() || !omit_empty_strings) {
    out->push_back(full.substr(start));
  }
}

std::vector<std::string> SplitUtf8(const std::string &text) {
  std::vector<std::string> result;
  size_t i = 0;
  
  while (i < text.length()) {
    unsigned char c = static_cast<unsigned char>(text[i]);
    size_t char_length = 1;
    
    // 判断UTF-8字符长度
    if (c >= 0xF0) char_length = 4;
    else if (c >= 0xE0) char_length = 3;
    else if (c >= 0xC0) char_length = 2;
    
    if (i + char_length <= text.length()) {
      result.push_back(text.substr(i, char_length));
    }
    i += char_length;
  }
  
  return result;
}

std::string ToLowerCase(const std::string &s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return result;
}

void ToLowerCase(std::string *in_out) {
  std::transform(in_out->begin(), in_out->end(), in_out->begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

std::string RemoveInvalidUtf8Sequences(const std::string &text) {
  // 简化版本，直接返回原文本
  // 在实际使用中可能需要更复杂的UTF-8验证
  return text;
}

}  // namespace hr_standalone
