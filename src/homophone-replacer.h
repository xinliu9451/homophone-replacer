// homophone-replacer.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef HOMOPHONE_REPLACER_H_
#define HOMOPHONE_REPLACER_H_

#include <memory>
#include <string>

namespace hr_standalone {

struct HomophoneReplacerConfig {
  std::string dict_dir;
  std::string lexicon;
  std::string rule_fsts;
  bool debug;

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
