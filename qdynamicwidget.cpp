#include "qdynamicwidget.h"

QDynamicWidget::QDynamicWidget(QWidget *parent) :
    QStackedWidget(parent)
{
    ResWtID++;                                                                          // Увеличение счетчика виджетов
    widgetID = ResWtID;                                                                 // Номер созданного виджета
}

QDynamicWidget::~QDynamicWidget()
{

}

int QDynamicWidget::getWtID()
{
    return widgetID;                                                                    // Возвращает номер виджета
}

int QDynamicWidget::ResWtID = 0;                                                        // Инициализация статической переменной класса
