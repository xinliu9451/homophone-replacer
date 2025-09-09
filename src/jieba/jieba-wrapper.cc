// jieba-wrapper.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "jieba-wrapper.h"
#include "../utils/file-utils.h"

#include "cppjieba/Jieba.hpp"
#include <iostream>

namespace hr_standalone {

JiebaWrapper::JiebaWrapper(const std::string &dict_dir) {
  if (dict_dir.empty()) {
    return;
  }

  std::string dict = dict_dir + "/jieba.dict.utf8";
  std::string hmm = dict_dir + "/hmm_model.utf8";
  std::string user_dict = dict_dir + "/user.dict.utf8";
  std::string idf = dict_dir + "/idf.utf8";
  std::string stop_word = dict_dir + "/stop_words.utf8";

  AssertFileExists(dict);
  AssertFileExists(hmm);
  AssertFileExists(user_dict);
  AssertFileExists(idf);
  AssertFileExists(stop_word);

  jieba_ = std::make_unique<cppjieba::Jieba>(dict, hmm, user_dict, idf, stop_word);
}

JiebaWrapper::~JiebaWrapper() = default;

void JiebaWrapper::Cut(const std::string &text, std::vector<std::string> &words, bool hmm) {
  if (jieba_) {
    jieba_->Cut(text, words, hmm);
  }
}

std::unique_ptr<JiebaWrapper> InitJieba(const std::string &dict_dir) {
  if (dict_dir.empty()) {
    return nullptr;
  }
  return std::make_unique<JiebaWrapper>(dict_dir);
}

}  // namespace hr_standalone
