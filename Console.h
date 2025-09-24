#ifndef CONSOLE_H
#define CONSOLE_H

#include "ConsoleInQt_global.h"
#include <iostream>
#include <string>
#include <QString>
#include <QDebug>
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
        return QString("background: %1; color: %2; caret-color: %3; font-size: %4px; border-width: 0; border-style: outset;")
                .arg(BackgroundColor, TextColor, TextCursorColor, TextSize);
    } //获取当前全局设置
};

class CONSOLEINQT_EXPORT Console : public QWidget
{
    Q_OBJECT

//私有方法
private:
    QVBoxLayout* CIQ_mainLayout=new QVBoxLayout(this); //主布局
    QScrollArea CIQ_scrollArea=QScrollArea(this); //滚动区域，隶属this
    QWidget CIQ_consoleWidget=QWidget(&CIQ_scrollArea); //控制台窗口，隶属CIQ_scrollArea
    QVBoxLayout* CIQ_consoleLayout=new QVBoxLayout(&CIQ_consoleWidget); //控制台窗口布局，隶属CIQ_consoleWidget

    // 用于焦点管理的私有方法
    void setFocusToCurrentInput() {
        if (CIQ_consoleLayout->count() > 0) {
            QWidget* lastWidget = CIQ_consoleLayout->itemAt(CIQ_consoleLayout->count()-1)->widget();
            if (lastWidget) {
                // 使用定时器确保在事件循环结束后设置焦点
                QTimer::singleShot(50, [lastWidget]() {
                    lastWidget->setFocus();
                    if (QLineEdit* input = qobject_cast<QLineEdit*>(lastWidget)) {
                        input->setCursorPosition(input->text().length());
                    }
                });
            }
        }
    }

    //初始化主布局和控制台窗口布局
    void initLayouts(void) {
        CIQ_mainLayout->setMargin(0);
        CIQ_mainLayout->setSpacing(0);
        CIQ_mainLayout->setContentsMargins(0,0,0,0);
        CIQ_mainLayout->setAlignment(Qt::AlignTop);
        CIQ_consoleLayout->setMargin(0);
        CIQ_consoleLayout->setSpacing(0);
        CIQ_consoleLayout->setContentsMargins(0,0,0,0);
        CIQ_consoleLayout->setAlignment(Qt::AlignTop);
    }

//公共方法
public:
    explicit Console(QWidget* parent = nullptr);
    ~Console() {
    }


    void keyPressEvent(QKeyEvent *event) override {
        //按键逻辑分支
        if (event->key() == Qt::Key_Return) {
            QLineEdit* currentInput = qobject_cast<QLineEdit*>(CIQ_consoleLayout->itemAt(CIQ_consoleLayout->count()-1)->widget());
            if (currentInput) {
                currentInput->setReadOnly(true);

                // 创建新输入框并添加到布局
                QLineEdit* newInput = new QLineEdit(&CIQ_consoleWidget);
                CIQ_consoleLayout->addWidget(newInput);

                // 设置新输入框焦点
                setFocusToCurrentInput();
            }
        }//针对按下回车键之后的事件处理机制
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

        CIQ_consoleWidget.setLayout(CIQ_consoleLayout);
        refreshAllStyleSheet(defaultConfig);

        // 添加初始输入框
        QLineEdit* initialInput = new QLineEdit(&CIQ_consoleWidget);
        CIQ_consoleLayout->addWidget(initialInput);

        // 设置焦点
        setFocusToCurrentInput();

        CIQ_scrollArea.setWidgetResizable(true);
        CIQ_scrollArea.setWidget(&CIQ_consoleWidget);

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

}
 // namespace ConsoleInQt
#endif // CONSOLE_H