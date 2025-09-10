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
<<<<<<< HEAD
#include <set>
=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7

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
<<<<<<< HEAD

    // 应用运行时增删规则（在已有 FST 基础上额外覆盖/屏蔽）
    BuildRuntimeRuleMap();
=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7
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

<<<<<<< HEAD
    // 在整句级别应用运行时词典覆盖（最后一步，优先级最高）
    if (!runtime_rule_map_.empty()) {
      result = ApplyRuntimeOverrides(result);
    }

=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7
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

<<<<<<< HEAD
  // 解析 config_ 中的 add_rules/del_rules/rules_file，构建运行时覆盖词典
  void BuildRuntimeRuleMap() {
    // del set
    std::set<std::string> del_keys(config_.del_rules.begin(), config_.del_rules.end());

    auto add_one = [&](const std::string &line) {
      auto pos = line.find('=');
      if (pos == std::string::npos) return;
      std::string key = line.substr(0, pos);
      std::string val = line.substr(pos + 1);
      if (key.empty() || val.empty()) return;
      if (del_keys.count(key)) return;  // 被删除的键不再添加
      runtime_rule_map_[key] = val;     // 覆盖
    };

    for (const auto &kv : config_.add_rules) add_one(kv);

    if (!config_.rules_file.empty()) {
      std::ifstream fin(config_.rules_file);
      if (fin) {
        std::string line;
        while (std::getline(fin, line)) {
          if (!line.empty()) add_one(line);
        }
      }
    }
  }

  // 在最终文本上按最长优先应用 runtime 规则：将拼音键替换为目标汉字
  std::string ApplyRuntimeOverrides(const std::string &text) const {
    if (runtime_rule_map_.empty()) return text;

    // 将文本转为拼音序列，再做子串替换，最后直接替换文本中的原片段。
    // 简化实现：直接做逐键查找替换（假设 keys 只会匹配由 ConvertWordToPronunciation 生成的拼音片段）。
    // 为稳妥，按 key 长度从长到短，避免较短键破坏较长匹配。
    std::vector<std::pair<std::string, std::string>> rules(runtime_rule_map_.begin(), runtime_rule_map_.end());
    std::sort(rules.begin(), rules.end(), [](auto &a, auto &b){return a.first.size() > b.first.size();});

    // 将整句再次按分词+拼音转为串，记录每个词的原文与拼音，用于定位替换
    bool is_hmm = true;
    std::vector<std::string> words;
    jieba_->Cut(text, words, is_hmm);

    std::vector<std::string> prons;
    prons.reserve(words.size());
    for (const auto &w : words) {
      if (w.size() < 3 || reinterpret_cast<const uint8_t *>(w.data())[0] < 128) {
        prons.push_back(w);
      } else {
        prons.push_back(ConvertWordToPronunciation(w));
      }
    }

    // 拼接为单一串以做匹配，并映射到词级范围
    std::string pron_seq;
    std::vector<size_t> word_start(prons.size());
    for (size_t i = 0; i < prons.size(); ++i) {
      word_start[i] = pron_seq.size();
      pron_seq += prons[i];
    }

    // 逐条规则替换：找到命中的子串，推回到词范围替换原文
    std::string out = text;
    for (const auto &kv : rules) {
      const std::string &key = kv.first;      // 拼音
      const std::string &val = kv.second;     // 目标汉字
      size_t pos = 0;
      while ((pos = pron_seq.find(key, pos)) != std::string::npos) {
        // 找到覆盖的词区间
        size_t begin_word = 0, end_word = prons.size();
        // 二分定位 begin_word
        begin_word = std::upper_bound(word_start.begin(), word_start.end(), pos) - word_start.begin();
        if (begin_word > 0) --begin_word;
        // 扩展到覆盖到 pos+key.size()
        size_t end_pos = pos + key.size();
        while (begin_word < prons.size() && word_start[begin_word] + prons[begin_word].size() <= pos) ++begin_word;
        size_t j = begin_word;
        while (j < prons.size() && word_start[j] < end_pos) ++j;
        end_word = j;
        if (begin_word >= end_word) { pos += 1; continue; }

        // 替换原文对应片段
        std::string before;
        for (size_t k = 0; k < begin_word; ++k) before += words[k];
        std::string after;
        for (size_t k = end_word; k < words.size(); ++k) after += words[k];
        out = before + val + after;

        // 更新 pron_seq 与 words/prons 以支持多次命中
        words.erase(words.begin() + begin_word, words.begin() + end_word);
        prons.erase(prons.begin() + begin_word, prons.begin() + end_word);
        words.insert(words.begin() + begin_word, val);
        prons.insert(prons.begin() + begin_word, val); // 目标词按原样插入

        // 重建 pron_seq 和 word_start
        pron_seq.clear();
        word_start.resize(prons.size());
        for (size_t t = 0; t < prons.size(); ++t) { word_start[t] = pron_seq.size(); pron_seq += prons[t]; }

        pos = 0; // 重新从头匹配该键，确保完全替换
      }
    }

    return out;
  }

=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7
 private:
  HomophoneReplacerConfig config_;
  std::unique_ptr<JiebaWrapper> jieba_;
  std::vector<std::unique_ptr<kaldifst::TextNormalizer>> replacer_list_;
  std::unordered_map<std::string, std::string> word2pron_;
<<<<<<< HEAD
  std::unordered_map<std::string, std::string> runtime_rule_map_;
=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7
};

HomophoneReplacer::HomophoneReplacer(const HomophoneReplacerConfig &config)
    : impl_(std::make_unique<Impl>(config)) {}

HomophoneReplacer::~HomophoneReplacer() = default;

std::string HomophoneReplacer::Apply(const std::string &text) const {
  return impl_->Apply(text);
}

}  // namespace hr_standalone
