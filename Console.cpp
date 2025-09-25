#include "Console.h"
using namespace ConsoleInQt;
Console::Console(QWidget* parent) : QWidget(parent)
{
    CIQ_consoleWidget.setFocusPolicy(Qt::StrongFocus);
    connect(CIQ_scrollArea.verticalScrollBar(),&QAbstractSlider::rangeChanged, [=]() {
            CIQ_scrollArea.verticalScrollBar()->setValue(CIQ_scrollArea.verticalScrollBar()->maximum());
        });
    installEventFilter(this);
}
