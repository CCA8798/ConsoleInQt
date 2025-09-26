#ifndef CONSOLE_H
#define CONSOLE_H

#include "ConsoleInQt_global.h"
#include <iostream>
#include <sstream>
#include <string>
#include <QString>
#include <QScrollBar>
#include <QDebug>
#include <QCoreApplication>
#include <QWidget>
#include <QScrollArea>
#include <QTextEdit>
#include <QTextLayout>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTimer> // 添加定时器用于焦点延迟设置

#define _CIQ_HEIGHT_MAX_ 16777215

namespace ConsoleInQt {

    // 控制台模式枚举
    enum class ConsoleModeEnum {
        Shell = 1 // Shell模式
    };

// 配置类，用于管理控制台的各种样式等配置
class CONSOLEINQT_EXPORT Config : public QObject {
    Q_OBJECT
public:
    // 构造函数
    explicit Config(QObject* parent = nullptr) {
        Q_UNUSED(parent);
    };
    // 析构函数
    ~Config() {};

protected:
    // =========^全局设置^=========//
    size_t ConsoleMode = static_cast<size_t>(ConsoleModeEnum::Shell); // 控制台模式
    QString WelcomeMessage = ""; // 欢迎消息
    QString BackgroundColor = "black"; // 背景颜色
    QString TextCursorColor = "white"; // 光标颜色
    QString TextColor = "white"; // 文本颜色
    QString TextSize = "20"; // 字体大小

public slots:
    // =========^全局设置方法^=========//
    // 设置欢迎消息
    inline void setWelcomeMessage(const QString& WelcomeMessageSc) { WelcomeMessage = WelcomeMessageSc; }
    // 获取欢迎消息
    inline QString getWelcomeMessage(void) const { return WelcomeMessage; };

    // 设置背景颜色
    inline void setBackgroundColor(const QString& BackgroundColorSc) { BackgroundColor = BackgroundColorSc; }
    // 获取背景颜色
    inline QString getBackgroundColor(void) const { return BackgroundColor; };

    // 设置文本颜色
    inline void setTextColor(const QString& TextColorSc) { TextColor = TextColorSc; }
    // 获取文本颜色
    inline QString getTextColor(void) const { return TextColor; }

    // 设置字体大小
    inline void setTextSize(const QString& TextSizeSc) { TextSize = TextSizeSc; }
    // 获取字体大小
    inline QString getTextSize(void) const { return TextSize; }

    // 测试输出，返回控制台模式
    inline size_t TestingOutput() const {
        return ConsoleMode;
    }

    // 获取当前全局设置，用于生成样式表
    inline QString getCurrentConfig(void) const {
        return QString("background: %1; color: %2; font-size: %3px; border-width: 0; border-style: outset;")
            .arg(BackgroundColor, TextColor, TextSize);
    }
};

class Console;
// 控制台缓冲区流类，用于处理要输出到控制台的各种类型数据并暂存
class CONSOLEINQT_EXPORT ConsoleBufferStream {
private:
    Console& console;
    std::string buffer;

    // 断言不支持的类型
    void assertUnsupportedValue() const {
        qWarning("UnsupportedValue(ConsoleInQt)");
    }
public:
    // 构造函数，关联到具体的Console对象
    explicit ConsoleBufferStream(Console& console) :console(console) {}
    // 禁用拷贝构造
    ConsoleBufferStream(const ConsoleBufferStream&) = delete;
    // 禁用拷贝赋值
    ConsoleBufferStream& operator=(const ConsoleBufferStream&) = delete;
    // 移动构造函数
    ConsoleBufferStream(ConsoleBufferStream&& other) noexcept
        : console(other.console), buffer(std::move(other.buffer)) {
        other.buffer.clear(); // 清空源对象缓冲区
    };
    // 析构函数声明
    inline ~ConsoleBufferStream();
    // 模板化的<<运算符重载，处理不同类型的数据到缓冲区
    template<typename BufferT = std::string_view>
    ConsoleBufferStream& operator<<(const BufferT& data) {
        if constexpr (std::is_convertible_v<std::decay_t<BufferT>, std::string_view> ||
            std::is_same_v<BufferT, std::string> ||
            std::is_same_v<BufferT, const char*>) {
            buffer += data;
        }
        else if constexpr (std::is_same_v<std::decay_t<BufferT>, QString>) {
            buffer += data.toStdString();
        }
        else if constexpr (std::is_arithmetic_v<BufferT>) {
            buffer += std::to_string(data);
        }
        else if constexpr (std::stringstream() << data) {
            std::stringstream strs;
            strs << data;
            buffer += strs.str();
        }
        else {
            assertUnsupportedValue();
        }
        return *this;
    }
};

// 控制台主类，继承自QWidget，实现控制台界面和交互
class CONSOLEINQT_EXPORT Console : public QWidget
{
    Q_OBJECT

    // 声明ConsoleBufferStream为友元类，使其能访问私有成员
    friend ConsoleBufferStream;

signals:
    // 命令发送信号，支持不同类型参数
    void commandSend(const std::string& command);
    void commandSend(const std::string_view& command);
    void commandSend(const QString& command);

// 私有方法
private:
    QGridLayout* CIQ_mainLayout = new QGridLayout(this); // 主布局
    QScrollArea CIQ_scrollArea = QScrollArea(this); // 滚动区域
    QWidget CIQ_consoleWidget = QWidget(&CIQ_scrollArea); // 控制台窗口
    QVBoxLayout* CIQ_consoleLayout = new QVBoxLayout(&CIQ_consoleWidget); // 控制台窗口布局
    QLineEdit* CIQ_currentLineEdit;

    // 用于焦点管理的私有方法，设置焦点到当前输入框
    void setFocusToCurrentInput() const {
        if (CIQ_consoleLayout->count() > 0) {
            if (QWidget* lastWidget = CIQ_consoleLayout->itemAt(CIQ_consoleLayout->count()-1)->widget()) {
                // 使用定时器确保在事件循环结束后设置焦点
                QTimer::singleShot(5, [lastWidget] {
                    lastWidget->setFocus();
                    if (auto* input = qobject_cast<QLineEdit*>(lastWidget)) {
                        input->setCursorPosition(input->text().length());
                    }
                });
            }
        }
    }

    // 在Shell模式下创建新行，content为行内容，userInput标识是否为用户输入行
    void CIQ_Shell_createNewLine(const QString& content = "", const bool userInput = true) {
        QLineEdit* currentInput = nullptr;
        if (CIQ_consoleLayout->count() > 0) {
            if ((currentInput = qobject_cast<QLineEdit*>(CIQ_consoleLayout->itemAt(CIQ_consoleLayout->count()-1)->widget()))) {
                currentInput->setReadOnly(true);
            }
        }
        auto* newInput = new QLineEdit(&CIQ_consoleWidget);
        newInput->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        CIQ_currentLineEdit = newInput;
        CIQ_consoleLayout->addWidget(newInput);
        if (userInput) {
            newInput->setText(">" + content);
            connect(CIQ_currentLineEdit, &QLineEdit::textChanged, [this]() {
                if (CIQ_currentLineEdit->text()[0] != ">") {
                    CIQ_currentLineEdit->setText(">" + CIQ_currentLineEdit->text());
                }
                });
            setFocusToCurrentInput();
        }
        else {
            newInput->setText(content);
            newInput->setReadOnly(true);
        }
    }

    // 初始化主布局和控制台窗口布局
    void initLayouts(void) {
        CIQ_consoleWidget.setLayout(CIQ_consoleLayout);
        CIQ_mainLayout->setMargin(0);
        CIQ_mainLayout->setSpacing(0);
        CIQ_mainLayout->setContentsMargins(0, 0, 0, 0);
        CIQ_mainLayout->setAlignment(Qt::AlignCenter);
        CIQ_consoleLayout->setMargin(0);
        CIQ_consoleLayout->setSpacing(0);
        CIQ_consoleLayout->setContentsMargins(0, 0, 0, 0);
        CIQ_consoleLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    }

    // 事件过滤器，处理事件
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (static_cast<QKeyEvent*>(event)->key() && CIQ_currentLineEdit != nullptr && CIQ_currentLineEdit->cursorPosition() <= 1) {
            CIQ_currentLineEdit->setCursorPosition(1);
        }
        return QWidget::eventFilter(watched, event);
    }

    // 按键事件处理
    void keyPressEvent(QKeyEvent* event) override {
        // 按键逻辑分支
        if (event->key() == Qt::Key_Return) {
            emit commandSend(std::string_view(std::string(std::string(CIQ_currentLineEdit->text().toStdString()).begin() + 1, std::string(CIQ_currentLineEdit->text().toStdString()).end())));
            QString command = "";
            for (int i = 1; i < CIQ_currentLineEdit->text().length(); i++) {
                command += CIQ_currentLineEdit->text()[i];
            }
            emit commandSend(command);
            emit commandSend(command.toStdString());
            CIQ_Shell_createNewLine();
            return;
        } // 针对按下回车键之后的事件处理机制
        if (event->key() && CIQ_currentLineEdit != nullptr && CIQ_currentLineEdit->cursorPosition() <= 1) {
            CIQ_currentLineEdit->setCursorPosition(1);
        }
    }
protected:
    // 处理缓冲区数据，将缓冲区内容按行输出到控制台
    void processBuffer(const std::string& buffer) {
        std::istringstream iss(buffer);
        std::string line;
        while (std::getline(iss, line)) {
            CIQ_Shell_createNewLine(QString::fromStdString(line), false);
        }
        CIQ_Shell_createNewLine();
    }

// 公共方法
public:
    // 构造函数
    explicit Console(QWidget* parent = nullptr);
    // 析构函数
    ~Console() {
    }

    // 模板化的<<运算符重载，用于创建ConsoleBufferStream处理输出内容
    template<typename OutputT = std::string_view>  // 默认以string_view构建
    ConsoleBufferStream operator<<(OutputT&& content) {
        ConsoleBufferStream bufferStream(*this);

        if constexpr (std::is_same_v<std::decay_t<OutputT>, QString>) {
            bufferStream << content.toStdString();
        }
        else if constexpr (std::is_convertible_v<OutputT, std::string_view>) {
            // 处理所有可转换为string_view的类型（包括string、char数组等）
            bufferStream << std::string_view(content);
        }
        else {
            // 回退到ostream处理（如数值类型）
            std::ostringstream oss;
            oss << content;
            bufferStream << oss.str();
        }

        return bufferStream;
    }

    // 刷新所有样式表，应用新的配置
    void refreshAllStyleSheet(const Config& config) {
        this->setStyleSheet(config.getCurrentConfig() + "QLineEdit{" + config.getCurrentConfig() + "};");
        CIQ_consoleWidget.setStyleSheet(config.getCurrentConfig() + "QLineEdit{" + config.getCurrentConfig() + "};");
    }

    // 初始化控制台，可指定默认配置和默认大小
    void init(const Config& defaultConfig = Config(), const QSize& defaultSize = QSize()) {
        initLayouts();
        // 大小调整
        if (!defaultSize.isNull()) {
            this->resize(defaultSize);
            CIQ_scrollArea.resize(this->size());
            CIQ_consoleWidget.resize(this->size());
        }
        refreshAllStyleSheet(defaultConfig);

        // 添加初始输入框
        if (!defaultConfig.getWelcomeMessage().isEmpty()) {
            *this << defaultConfig.getWelcomeMessage();
        }
        else {
            CIQ_Shell_createNewLine();
        }

        CIQ_scrollArea.setWidget(&CIQ_consoleWidget);
        CIQ_scrollArea.setWidgetResizable(true);
        CIQ_scrollArea.setAlignment(Qt::AlignLeft);
        CIQ_mainLayout->addWidget(&CIQ_scrollArea);
        CIQ_scrollArea.show();
        CIQ_consoleWidget.show();
        this->show();
    }

    // 重写 resizeEvent 处理窗口大小调整
    inline void resizeEvent(QResizeEvent*) override {
        CIQ_consoleWidget.resize(this->size());
    }

public slots:
    // Shell Mode相关槽函数（此处暂空，可后续扩展）
};

// ConsoleBufferStream析构函数定义，在析构时处理缓冲区数据
inline ConsoleBufferStream::~ConsoleBufferStream() {
    console.processBuffer(buffer);
}

} // namespace ConsoleInQt
#endif // CONSOLE_H