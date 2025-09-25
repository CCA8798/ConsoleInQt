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

    //控制台模式枚举
    enum class ConsoleModeEnum {
        Shell=1, //Shell模式
        Text=2 //普通文本模式
    };

class CONSOLEINQT_EXPORT Config : public QObject{
    Q_OBJECT
public:
    explicit Config(QObject* parent=nullptr){
        Q_UNUSED(parent);
    };
    ~Config(){};

protected:

    //=========^全局设置^=========//
    size_t ConsoleMode=static_cast<size_t>(ConsoleModeEnum::Shell); //控制台模式
    QString WelcomeMessage=""; //欢迎消息
    QString BackgroundColor="black"; //背景颜色
    QString TextCursorColor="white"; //光标颜色
    QString TextColor="white"; //文本颜色
    QString TextSize="20"; //字体大小

public slots:

    //=========^全局设置方法^=========//
    inline void setWelcomeMessage(const QString& WelcomeMessageSc){WelcomeMessage=WelcomeMessageSc;}
    inline QString getWelcomeMessage(void) const{return WelcomeMessage;};

    inline void setBackgroundColor(const QString& BackgroundColorSc){BackgroundColor=BackgroundColorSc;}
    inline QString getBackgroundColor(void) const{return BackgroundColor;};

    inline void setTextColor(const QString& TextColorSc){TextColor=TextColorSc;}
    inline QString getTextColor(void) const{return TextColor;}

    inline void setTextSize(const QString& TextSizeSc){TextSize=TextSizeSc;}
    inline QString getTextSize(void) const{return TextSize;}

    inline size_t TestingOutput() const{
        return ConsoleMode;
    } //测试输出

    inline QString getCurrentConfig(void) const {
        return QString("background: %1; color: %2; font-size: %3px; border-width: 0; border-style: outset;")
                .arg(BackgroundColor, TextColor, TextSize);
    } //获取当前全局设置
};


class Console;
class CONSOLEINQT_EXPORT ConsoleBufferStream {
private:
    Console& console;
    std::string buffer;
public:
    explicit ConsoleBufferStream(Console& console):console(console){}
    ConsoleBufferStream(const ConsoleBufferStream&)=delete;
    ConsoleBufferStream& operator=(const ConsoleBufferStream&)=delete;
    ConsoleBufferStream(ConsoleBufferStream&& other) noexcept
    : console(other.console), buffer(std::move(other.buffer)) {
            other.buffer.clear(); // 清空源对象缓冲区
        };
    inline ~ConsoleBufferStream();
    template <typename BufferT>
   ConsoleBufferStream& operator<<(const BufferT& data) {
        std::ostringstream oss;
        oss<<data;
        buffer += oss.str(); // 追加到缓冲区
        return *this;
    }
};

class CONSOLEINQT_EXPORT Console : public QWidget
{
    Q_OBJECT

    friend ConsoleBufferStream;

    signals:
    void commandSend(const std::string_view& command);
    void commandSend(const QString& command);

//私有方法
private:
    QGridLayout* CIQ_mainLayout=new QGridLayout(this); //主布局
    QScrollArea CIQ_scrollArea=QScrollArea(this); //滚动区域，隶属this
    QWidget CIQ_consoleWidget=QWidget(&CIQ_scrollArea); //控制台窗口，隶属CIQ_scrollArea
    QVBoxLayout* CIQ_consoleLayout=new QVBoxLayout(&CIQ_consoleWidget); //控制台窗口布局，隶属CIQ_consoleWidget
    QLineEdit* CIQ_currentLineEdit;

    // 用于焦点管理的私有方法
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
                //QThread::sleep(50);
                /*lastWidget->setFocus();
                if (auto* input = qobject_cast<QLineEdit*>(lastWidget)) {
                    input->setCursorPosition(input->text().length());
                }*/
            }
        }
    }

    void CIQ_Shell_createNewLine(const QString& content="",const bool userInput=true) {
        QLineEdit* currentInput = nullptr;
        if (CIQ_consoleLayout->count()>0) {
            if ( (currentInput = qobject_cast<QLineEdit*>(CIQ_consoleLayout->itemAt(CIQ_consoleLayout->count()-1)->widget()))) {
                currentInput->setReadOnly(true);
            }
        }
        auto* newInput = new QLineEdit(&CIQ_consoleWidget);
        newInput->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        CIQ_currentLineEdit=newInput;
        CIQ_consoleLayout->addWidget(newInput);
        if (userInput) {
            newInput->setText(">"+content);
            connect(CIQ_currentLineEdit, &QLineEdit::textChanged, [this](){
        if (CIQ_currentLineEdit->text()[0]!=">") {
          CIQ_currentLineEdit->setText(">"+CIQ_currentLineEdit->text());
          }});
            setFocusToCurrentInput();
        }
        else {
            newInput->setText(content);
            newInput->setReadOnly(true);
        }
    }

    //初始化主布局和控制台窗口布局
    void initLayouts(void) {
        CIQ_consoleWidget.setLayout(CIQ_consoleLayout);
        CIQ_mainLayout->setMargin(0);
        CIQ_mainLayout->setSpacing(0);
        CIQ_mainLayout->setContentsMargins(0,0,0,0);
        CIQ_mainLayout->setAlignment(Qt::AlignCenter);
        CIQ_consoleLayout->setMargin(0);
        CIQ_consoleLayout->setSpacing(0);
        CIQ_consoleLayout->setContentsMargins(0,0,0,0);
        CIQ_consoleLayout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    }

    bool eventFilter(QObject *watched, QEvent *event) override {
        if (static_cast<QKeyEvent*>(event)->key()&&CIQ_currentLineEdit!=nullptr&&CIQ_currentLineEdit->cursorPosition()<=1) {
            CIQ_currentLineEdit->setCursorPosition(1);
        }
        return QWidget::eventFilter(watched, event);
    }

    void keyPressEvent(QKeyEvent *event) override {
        //按键逻辑分支
        if (event->key() == Qt::Key_Return) {
            emit commandSend(std::string_view(std::string(std::string(CIQ_currentLineEdit->text().toStdString()).begin()+1,std::string(CIQ_currentLineEdit->text().toStdString()).end())));
            QString command="";
            for (int i=1;i<CIQ_currentLineEdit->text().length();i++) {
                command+=CIQ_currentLineEdit->text()[i];
            }
            emit commandSend(command);
            CIQ_Shell_createNewLine();
            return;
        }//针对按下回车键之后的事件处理机制
        if (event->key()&&CIQ_currentLineEdit!=nullptr&&CIQ_currentLineEdit->cursorPosition()<=1) {
            CIQ_currentLineEdit->setCursorPosition(1);
        }
    }
protected:
    void processBuffer(const std::string& buffer) {
        std::istringstream iss(buffer);
        std::string line;
        while (std::getline(iss, line)) {
            CIQ_Shell_createNewLine(QString::fromStdString(line), false);
        }
        CIQ_Shell_createNewLine();
    }

//公共方法
public:
    explicit Console(QWidget* parent = nullptr);
    ~Console() {
    }

    template<typename OutputT = std::string_view>  // 默认以string_view构建
    ConsoleBufferStream operator<<(OutputT&& content) {  
        ConsoleBufferStream bufferStream(*this);

        if constexpr(std::is_same_v<std::decay_t<OutputT>, QString>) {
            bufferStream << content.toStdString();
        }
        else if constexpr(std::is_convertible_v<OutputT, std::string_view>) {
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

    void refreshAllStyleSheet(const Config& config) {
        this->setStyleSheet(config.getCurrentConfig()+"QLineEdit{"+config.getCurrentConfig()+"};");
        CIQ_consoleWidget.setStyleSheet(config.getCurrentConfig()+"QLineEdit{"+config.getCurrentConfig()+"};");
    }

    void init(const Config& defaultConfig=Config(),const QSize& defaultSize=QSize()) {
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
            *this<<defaultConfig.getWelcomeMessage();
        }else {
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

    inline void resizeEvent(QResizeEvent*) override{
        CIQ_consoleWidget.resize(this->size());
    }

public slots:
    // Shell Mode
};

inline ConsoleBufferStream::~ConsoleBufferStream() {
    console.processBuffer(buffer);
}


}
 // namespace ConsoleInQt
#endif // CONSOLE_H