@echo off
rem Windows构建脚本 for homophone-replacer-standalone
rem 需要Visual Studio 2019或更高版本

echo ====================================================
echo Building Homophone Replacer Standalone for Windows
echo ====================================================

rem 检查是否在VS开发者命令提示符中
if "%VCINSTALLDIR%"=="" (
    echo Error: Please run this script from Visual Studio Developer Command Prompt
    echo 请在Visual Studio开发者命令提示符中运行此脚本
    pause
    exit /b 1
)

rem 创建构建目录
if not exist build mkdir build
cd build

rem 检查cppjieba是否存在
if not exist ..\third_party\cppjieba\include\cppjieba\Jieba.hpp (
    echo Error: cppjieba not found in third_party directory
    echo 错误：在third_party目录中找不到cppjieba
    echo Please download cppjieba and extract to third_party/cppjieba/
    echo 请下载cppjieba并解压到third_party/cppjieba/目录
    pause
    exit /b 1
)

rem 运行CMake配置
echo Running CMake configuration...
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    echo CMake配置失败！
    pause
    exit /b 1
)

rem 编译项目
echo Building project...
cmake --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo Build failed!
    echo 编译失败！
    pause
    exit /b 1
)

echo.
echo ====================================================
echo Build completed successfully!
echo 编译成功完成！
echo.
echo Executable location: build\bin\Release\homophone-replacer-standalone.exe
echo 可执行文件位置: build\bin\Release\homophone-replacer-standalone.exe
echo.
echo Usage example:
echo 使用示例:
echo   cd build\bin\Release
echo   homophone-replacer-standalone.exe --text "他想知道这个问题的答案"
echo ====================================================

pause
