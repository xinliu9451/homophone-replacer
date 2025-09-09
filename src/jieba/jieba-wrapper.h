// jieba-wrapper.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef JIEBA_WRAPPER_H_
#define JIEBA_WRAPPER_H_

#include <memory>
#include <string>
#include <vector>

// 前向声明，避免直接包含cppjieba头文件
namespace cppjieba {
class Jieba;
}

namespace hr_standalone {

class JiebaWrapper {
 public:
  explicit JiebaWrapper(const std::string &dict_dir);
  ~JiebaWrapper();

  // 分词
  void Cut(const std::string &text, std::vector<std::string> &words, bool hmm = true);

 private:
  std::unique_ptr<cppjieba::Jieba> jieba_;
};

// 初始化Jieba实例
std::unique_ptr<JiebaWrapper> InitJieba(const std::string &dict_dir);

}  // namespace hr_standalone

#endif  // JIEBA_WRAPPER_H_
