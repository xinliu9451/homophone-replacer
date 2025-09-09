# Homophone Replacer Standalone

这是一个从sherpa-onnx项目中独立出来的拼音词组替换工具，专门用于中文同音字的智能替换。

## 功能特点

- 独立的C++项目，无需完整的sherpa-onnx环境
- 支持Windows和Linux平台编译
- 基于 jieba 分词 + kaldifst FST 规则（replace.fst）的同音词组替换
- 最小化依赖，快速编译和部署

## 项目结构

```
homophone-replacer-standalone/
├── src/                    # 源代码
│   ├── homophone-replacer.h/cc  # 核心同音字替换类
│   ├── main.cc                 # 主程序入口
│   ├── jieba/                  # jieba分词封装
│   └── utils/                  # 工具函数
├── data/hr-files/              # 数据文件
│   ├── dict/                   # jieba词典文件
│   ├── lexicon.txt             # 拼音词典
│   └── replace.fst             # 替换规则（FST）
├── third_party/                # 第三方依赖
│   └── cppjieba/               # cppjieba库
├── CMakeLists.txt              # CMake配置
├── build.bat                   # Windows构建脚本
├── build.sh                    # Linux构建脚本
└── README.md                   # 本文件
```

## 快速开始

### 1. 准备环境

**Windows:**
- Visual Studio 2019或更高版本
- CMake 3.13+

**Linux:**
- GCC 7+ 或 Clang 8+
- CMake 3.13+

### 2. 检查依赖

cppjieba已经自动下载到`third_party/cppjieba/`目录。

### 3. 编译

**Windows:**
```cmd
# 在VS开发者命令提示符中
build.bat
```

**Linux:**
```bash
chmod +x build.sh
# 默认 Release
./build.sh

# Debug
CONFIG=Debug ./build.sh

# 使用 Ninja
GENERATOR="Ninja" ./build.sh

# 指定构建目录
BUILD_DIR=out ./build.sh

# 运行（如遇共享库找不到，先设置环境变量）
export LD_LIBRARY_PATH="./build/lib:$LD_LIBRARY_PATH"
./build/bin/homophone-replacer-standalone --text "他想知道玄界芯片问题的答案" --debug
```

### 4. 使用

```bash
# 基本使用
./homophone-replacer-standalone --text "他想知道玄界芯片问题的答案"

# 启用调试信息
./homophone-replacer-standalone --text "他想知道答案" --debug

# 查看帮助
./homophone-replacer-standalone --help
```

## 输入输出示例

```
输入: "他想知道玄界芯片问题的答案"
输出: "他想知道玄戒芯片问题的答案"
```

## 技术原理

1. **分词**: 使用 jieba 对输入文本进行中文分词
2. **拼音转换**: 词语转为拼音序列（来自 `lexicon.txt`，单字回退）
3. **FST 规则重写**: 使用 `replace.fst` 在拼音序列上进行短语级匹配与替换
4. **结果重构**: 根据 FST 输出的词序列重组为文本

## 自定义扩展

要自定义短语替换：
- 使用官方 Colab 脚本生成新的 `replace.fst`。[👉 点击运行](https://colab.research.google.com/drive/1jEaS3s8FbRJIcVQJv2EQx19EM_mnuARi?usp=sharing)
- 将生成的 `replace.fst` 覆盖到 `data/hr-files/replace.fst`

注意：当前仅支持单个 `replace.fst` 文件
