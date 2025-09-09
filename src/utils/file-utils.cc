// file-utils.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "file-utils.h"

#include <fstream>
#include <iostream>

namespace hr_standalone {

bool FileExists(const std::string &filename) {
  return std::ifstream(filename).good();
}

void AssertFileExists(const std::string &filename) {
  if (!FileExists(filename)) {
    std::cerr << "Error: file '" << filename << "' does not exist" << std::endl;
    exit(-1);
  }
}

std::vector<char> ReadFile(const std::string &filename) {
  std::ifstream input(filename, std::ios::binary);
  std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
  return buffer;
}

}  // namespace hr_standalone
