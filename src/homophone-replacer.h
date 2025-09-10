// homophone-replacer.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef HOMOPHONE_REPLACER_H_
#define HOMOPHONE_REPLACER_H_

#include <memory>
#include <string>
<<<<<<< HEAD
#include <vector>
=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7

namespace hr_standalone {

struct HomophoneReplacerConfig {
  std::string dict_dir;
  std::string lexicon;
  std::string rule_fsts;
  bool debug;

<<<<<<< HEAD
  // 运行时拼音规则：格式 "pinyin=汉字"，如 "pai2zhang3=排长"
  std::vector<std::string> add_rules;
  // 删除规则：仅拼音键，如 "pai2zhang3"
  std::vector<std::string> del_rules;
  // 从文件批量加载规则（每行同 add_rules 的格式）
  std::string rules_file;

=======
>>>>>>> ac85ce3960f2c6e6d6fd709f483c882ac7f153e7
  HomophoneReplacerConfig() = default;

  HomophoneReplacerConfig(const std::string &dict_dir,
                          const std::string &lexicon,
                          const std::string &rule_fsts, bool debug)
      : dict_dir(dict_dir),
        lexicon(lexicon),
        rule_fsts(rule_fsts),
        debug(debug) {}

  bool Validate() const;
  std::string ToString() const;
};

class HomophoneReplacer {
 public:
  explicit HomophoneReplacer(const HomophoneReplacerConfig &config);
  ~HomophoneReplacer();

  std::string Apply(const std::string &text) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace hr_standalone

#endif  // HOMOPHONE_REPLACER_H_
