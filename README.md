[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build System](https://img.shields.io/badge/Build-CMake-blue)](https://cmake.org)
[![Qt Version](https://img.shields.io/badge/Qt_Version->=5.12-green)](https://www.qt.io/download)
[![C++ Standard](https://img.shields.io/badge/C++_Standard->=C++17-green)](https://cppreference.com)
# ConsoleInQt

一个轻量级、可自定义的 Qt 应用程序控制台组件，旨在模拟类 Shell 交互体验，支持多种输入/输出数据类型。


## 项目概述

ConsoleInQt 是一款基于 Qt 框架开发的控制台组件，可无缝集成到 Qt 应用中，提供类似终端的交互界面。适用于开发调试控制台、自定义 Shell 工具或需要命令行交互的应用场景，支持命令输入、文本输出及灵活的样式定制。

核心包含三大模块：
- **Console 核心组件**：实现控制台界面渲染与用户交互逻辑
- **ConsoleBufferStream 流处理**：支持多类型数据的安全输出与缓冲管理
- **Config 配置类**：统一管理控制台样式、欢迎信息等可定制参数


## 核心特性

- **多类型数据支持**：原生处理字符串（`std::string`/`const char*`）、Qt 字符串（`QString`）、数值类型（整数/浮点数）
- **高度可定制化**：支持自定义背景色、文本色、字体大小、欢迎消息，一键应用样式
- **类 Shell 交互**：命令提示符（`>`）自动添加，禁止删除提示符，回车触发命令发送
- **Qt 信号槽集成**：命令输入通过信号（`commandSend`）对外传递，轻松对接业务逻辑
- **自动焦点管理**：新行创建后自动将焦点定位到输入框末尾，优化交互体验
- **滚动区域适配**：内置滚动视图，支持大量文本输出时的滚动查看


## 安装与集成

### 前置依赖
- Qt 5.14.2 及以上版本（支持 MinGW 7.3 / MSVC 2019 编译）
- CMake 3.10 及以上
- C++17 兼容编译器

### 集成步骤
1. 将 `Console.h`、`Console.cpp` 及 `ConsoleInQt_global.h` 加入项目
2. 在 Qt 项目文件（`.pro`）中添加依赖：
   ```qmake
   QT += widgets
   SOURCES += Console.cpp
   HEADERS += Console.h ConsoleInQt_global.h
   ```
3. 若使用 CMake 构建，参考项目自带的 `CMakeLists.txt` 配置编译选项


## 快速使用示例

```cpp
#include "Console.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // 1. 创建并配置控制台参数
    ConsoleInQt::Config consoleConfig;
    consoleConfig.setWelcomeMessage("欢迎使用 ConsoleInQt 控制台！\n输入命令并按回车执行");
    consoleConfig.setBackgroundColor("black");    // 黑色背景
    consoleConfig.setTextColor("lightgreen");     // 亮绿色文本
    consoleConfig.setTextSize("14");              // 14号字体
    
    // 2. 创建控制台实例并初始化
    ConsoleInQt::Console console;
    console.init(consoleConfig, QSize(800, 600)); // 初始化并设置窗口大小（800x600）
    
    // 3. 连接命令信号（处理用户输入的命令）
    QObject::connect(&console, &ConsoleInQt::Console::commandSend,
                     [&console](const QString& command) {
        // 示例：echo 命令处理（输出用户输入的内容）
        if (command.startsWith("echo ")) {
            QString content = command.mid(5); // 截取 "echo " 后的内容
            console << "[输出] " << content << "\n"; // 输出到控制台
        }
        // 示例：clear 命令处理（清空控制台，需自行实现清空逻辑）
        else if (command == "clear") {
            console << "[提示] 控制台已清空\n";
        }
        // 未知命令提示
        else {
            console << "[错误] 未知命令：" << command << "\n";
        }
    });
    
    // 4. 主动输出文本到控制台
    console << "控制台初始化完成！\n";
    console << "支持命令：echo <内容>（输出文本）、clear（清空）\n";
    
    return a.exec();
}
```


## 配置说明（Config 类）

通过 `Config` 类可灵活调整控制台外观与初始行为，核心接口如下：

| 方法                          | 功能描述                                  | 参数示例                |
|-------------------------------|-------------------------------------------|-------------------------|
| `setWelcomeMessage(const QString&)` | 设置控制台启动时的欢迎消息                | `"Welcome to Console!"` |
| `setBackgroundColor(const QString&)` | 设置控制台背景色（支持 CSS 颜色格式）     | `"#1a1a1a"`（深灰）     |
| `setTextColor(const QString&)`       | 设置控制台文本颜色                        | `"white"`（白色）       |
| `setTextSize(const QString&)`        | 设置字体大小（单位：像素）                | `"16"`                  |
| `getCurrentConfig()`                 | 获取当前配置的样式表字符串（供 Qt 使用）  | -                       |


## 核心接口（Console 类）

| 接口                          | 功能描述                                  | 使用示例                          |
|-------------------------------|-------------------------------------------|-----------------------------------|
| `init(const Config&, const QSize&)` | 初始化控制台（应用配置+设置窗口大小）     | `console.init(config, QSize(800,600))` |
| `operator<<(T content)`       | 输出数据到控制台（支持多类型）            | `console << "Version: " << 1.0 << "\n";` |
| `refreshAllStyleSheet(const Config&)` | 重新应用配置（动态更新样式）              | `console.refreshAllStyleSheet(newConfig)` |


## 信号说明

Console 类通过信号对外传递用户输入的命令，支持三种参数类型，可根据需求选择连接：

| 信号                          | 功能描述                                  | 连接示例                          |
|-------------------------------|-------------------------------------------|-----------------------------------|
| `commandSend(const std::string&)` | 传递命令为 `std::string` 类型             | `connect(&c, &Console::commandSend, [](const std::string& cmd){...})` |
| `commandSend(const QString&)`     | 传递命令为 `QString` 类型（Qt 推荐）      | `connect(&c, &Console::commandSend, [](const QString& cmd){...})` |
| `commandSend(const std::string_view&)` | 传递命令为 `std::string_view` 类型（高效） | `connect(&c, &Console::commandSend, [](const std::string_view& cmd){...})` |


## 构建与产物

### 支持的编译环境
- **编译器**：MinGW 7.3（64位）、MSVC 2019（64位）
- **构建类型**：Debug（调试版，含调试信息）、Release（发布版，优化大小/性能）
- **输出产物**：动态链接库（`.dll`）、导入库（`.lib`，MSVC 环境）、头文件（`.h`）

### 编译说明
若使用提供的 GitHub Actions 脚本，可自动构建 4 个版本的产物：
- MinGW 7.3 - Debug
- MinGW 7.3 - Release
- MSVC 2019 - Debug
- MSVC 2019 - Release

## 许可证

本项目采用 **MIT 许可证** 开源，允许个人/商业使用、修改、分发，无需支付授权费用，详情见 [LICENSE](LICENSE) 文件。


## 致谢

- 基于 Qt 框架开发，感谢 Qt 团队提供的跨平台 UI 解决方案
- 灵感来源于标准终端工具，旨在为 Qt 应用提供更便捷的命令交互能力

## 注意事项

1. 若需支持更多数据类型，可扩展 `ConsoleBufferStream` 的 `operator<<` 模板重载
2. 动态调整窗口大小时，控制台内容会自动适配（通过重写 `resizeEvent` 实现）
3. Debug 版本产物含调试信息，体积较大；Release 版本经过优化，适合部署使用
4. 若需添加命令历史记录、语法高亮等功能，可基于现有代码扩展 `Console` 类

*Note: This project is still in development. Contributions and feedback are welcome!*
