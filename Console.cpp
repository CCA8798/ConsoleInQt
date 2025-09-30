//Console.cpp
#include "Console.h"
using namespace ConsoleInQt;
Console::Console(QWidget* parent,const Config& defaultConfig, const QSize& defaultSize) : QWidget(parent)
{
    if (CIQ_consoleLayout==nullptr||CIQ_mainLayout==nullptr) {
        throw std::bad_alloc();
    }
    initLayouts();
    // 大小调整
    if (!defaultSize.isNull()) {
        this->resize(defaultSize);
        CIQ_scrollArea.resize(this->size());
        CIQ_consoleWidget.resize(this->size());
    }
    refreshAllStyleSheet(defaultConfig);

    CIQ_scrollArea.setWidget(&CIQ_consoleWidget);
    CIQ_scrollArea.setWidgetResizable(true);
    CIQ_scrollArea.setAlignment(Qt::AlignLeft);
    CIQ_mainLayout->addWidget(&CIQ_scrollArea);
    CIQ_scrollArea.show();
    CIQ_consoleWidget.show();
    this->show();
    CIQ_consoleWidget.setFocusPolicy(Qt::StrongFocus);
    connect(CIQ_scrollArea.verticalScrollBar(),&QAbstractSlider::rangeChanged, [=]() {
            CIQ_scrollArea.verticalScrollBar()->setValue(CIQ_scrollArea.verticalScrollBar()->maximum());
        });
    installEventFilter(this);

    // 添加初始输入框
    if (!defaultConfig.getWelcomeMessage().isEmpty()) {
        *this << defaultConfig.getWelcomeMessage();
    }
    else {
        CIQ_Shell_createNewLine();
    }
}
