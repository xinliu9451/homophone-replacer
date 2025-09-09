// homophone-replacer.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "homophone-replacer.h"
#include "utils/file-utils.h"
#include "utils/text-utils.h"
#include "jieba/jieba-wrapper.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <iomanip>

// 引入 FST 文本归一化器
#include "kaldifst/csrc/text-normalizer.h"

namespace hr_standalone {

bool HomophoneReplacerConfig::Validate() const {
  if (!dict_dir.empty()) {
    std::vector<std::string> required_files = {
        "jieba.dict.utf8", "hmm_model.utf8",  "user.dict.utf8",
        "idf.utf8",        "stop_words.utf8",
    };

    for (const auto &f : required_files) {
      if (!FileExists(dict_dir + "/" + f)) {
        std::cerr << "Error: '" << dict_dir << "/" << f 
                  << "' does not exist. Please check dict-dir" << std::endl;
        return false;
      }
    }
  }

  if (!lexicon.empty() && !FileExists(lexicon)) {
    std::cerr << "Error: lexicon file '" << lexicon << "' does not exist" << std::endl;
    return false;
  }

  if (!rule_fsts.empty()) {
    std::vector<std::string> files;
    SplitStringToVector(rule_fsts, ",", false, &files);

    if (files.size() > 1) {
      std::cerr << "Error: Only 1 FST file is supported now." << std::endl;
      return false;
    }

    for (const auto &f : files) {
      if (!FileExists(f)) {
        std::cerr << "Error: Rule fst '" << f << "' does not exist." << std::endl;
        return false;
      }
    }
  }

  return true;
}

std::string HomophoneReplacerConfig::ToString() const {
  std::ostringstream os;
  os << "HomophoneReplacerConfig(";
  os << "dict_dir=\"" << dict_dir << "\", ";
  os << "lexicon=\"" << lexicon << "\", ";
  os << "rule_fsts=\"" << rule_fsts << "\")";
  return os.str();
}

class HomophoneReplacer::Impl {
 public:
  explicit Impl(const HomophoneReplacerConfig &config) : config_(config) {
    jieba_ = InitJieba(config.dict_dir);

    if (!config.lexicon.empty()) {
      std::ifstream is(config.lexicon);
      InitLexicon(is);
    }

    // 加载 FST 规则
    if (!config.rule_fsts.empty()) {
      std::vector<std::string> files;
      SplitStringToVector(config.rule_fsts, ",", false, &files);
      replacer_list_.reserve(files.size());
      for (const auto &f : files) {
        if (config.debug) {
          std::cout << "hr rule fst: " << f << std::endl;
        }
        replacer_list_.push_back(std::make_unique<kaldifst::TextNormalizer>(f));
      }
    }
  }

  std::string Apply(const std::string &text) const {
    if (text.empty() || !jieba_) {
      return text;
    }

    auto now = [](){ return std::chrono::steady_clock::now(); };
    auto t0 = now();

    bool is_hmm = true;
    std::vector<std::string> words;
    jieba_->Cut(text, words, is_hmm);

    auto t1 = now();

    if (config_.debug) {
      std::cout << "Input text: '" << text << "'" << std::endl;
      std::ostringstream os;
      os << "After jieba: ";
      std::string sep;
      for (const auto &w : words) {
        os << sep << w;
        sep = "_";
      }
      std::cout << os.str() << std::endl;
    }

    // 将连续中文片段聚合，按段应用 FST 规则
    std::string result;
    std::vector<std::string> current_words;
    std::vector<std::string> current_pronunciations;

    auto flush_segment = [&](std::string &out) {
      if (current_words.empty()) return;
      out += ApplyImpl(current_words, current_pronunciations);
      current_words.clear();
      current_pronunciations.clear();
    };

    auto t2 = now();

    for (const auto &w : words) {
      // 非中文（或长度不足一个中文字符）作为分隔，先冲刷累计段
      if (w.size() < 3 || reinterpret_cast<const uint8_t *>(w.data())[0] < 128) {
        flush_segment(result);
        result += w;
        continue;
      }

      auto p = ConvertWordToPronunciation(w);
      if (config_.debug) {
        std::cout << w << " -> " << p << std::endl;
      }
      current_words.push_back(w);
      current_pronunciations.push_back(std::move(p));
    }

    auto t3 = now();

    flush_segment(result);

    auto t4 = now();

    if (config_.debug) {
      auto ms = [](auto a, auto b) {
        return std::chrono::duration<double, std::milli>(b - a).count();
      };
      std::cout << std::fixed << std::setprecision(4)
                << "Timing(ms): seg=" << ms(t0, t1)
                << ", prep=" << ms(t1, t2)
                << ", pinyin=" << ms(t2, t3)
                << ", fst=" << ms(t3, t4)
                << std::endl;
      std::cout << "Output text: '" << result << "'" << std::endl;
    }

    return RemoveInvalidUtf8Sequences(result);
  }

 private:
  // 应用 FST 规则到当前中文片段
  std::string ApplyImpl(const std::vector<std::string> &words,
                        const std::vector<std::string> &pronunciations) const {
    if (!replacer_list_.empty()) {
      for (const auto &r : replacer_list_) {
        // 目前仅支持一个规则文件
        return r->Normalize(words, pronunciations);
      }
    }
    // 没有 FST 规则时，直接拼接原词
    std::string ans;
    for (const auto &w : words) ans.append(w);
    return ans;
  }
  std::string ConvertWordToPronunciation(const std::string &word) const {
    if (word2pron_.count(word)) {
      return word2pron_.at(word);
    }

    if (word.size() <= 3) {
      return word;
    }

    std::vector<std::string> chars = SplitUtf8(word);
    std::string ans;
    for (const auto &c : chars) {
      if (word2pron_.count(c)) {
        ans.append(word2pron_.at(c));
      } else {
        ans.append(c);
      }
    }

    return ans;
  }

  void InitLexicon(std::istream &is) {
    std::string word;
    std::string pron;
    std::string p;
    std::string line;
    int32_t line_num = 0;
    int32_t num_warn = 0;
    
    while (std::getline(is, line)) {
      ++line_num;
      std::istringstream iss(line);

      pron.clear();
      iss >> word;
      ToLowerCase(&word);

      if (word2pron_.count(word)) {
        num_warn += 1;
        if (num_warn < 10) {
          std::cerr << "Warning: Duplicated word: " << word 
                    << " at line " << line_num << ":" << line 
                    << ". Ignore it." << std::endl;
        }
        continue;
      }

      while (iss >> p) {
        if (p.back() > '4') {
          p.push_back('1');
        }
        pron.append(std::move(p));
      }

      if (pron.empty()) {
        std::cerr << "Warning: Empty pronunciation for word '" << word 
                  << "' at line " << line_num << ":" << line 
                  << ". Ignore it." << std::endl;
        continue;
      }

      word2pron_.insert({std::move(word), std::move(pron)});
    }
  }

 private:
  HomophoneReplacerConfig config_;
  std::unique_ptr<JiebaWrapper> jieba_;
  std::vector<std::unique_ptr<kaldifst::TextNormalizer>> replacer_list_;
  std::unordered_map<std::string, std::string> word2pron_;
};

HomophoneReplacer::HomophoneReplacer(const HomophoneReplacerConfig &config)
    : impl_(std::make_unique<Impl>(config)) {}

HomophoneReplacer::~HomophoneReplacer() = default;

std::string HomophoneReplacer::Apply(const std::string &text) const {
  return impl_->Apply(text);
}

}  // namespace hr_standalone
