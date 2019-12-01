#ifndef QDYNAMICWIDGET_H
#define QDYNAMICWIDGET_H

#include <QStackedWidget>

class QDynamicWidget : public QStackedWidget
{

    Q_OBJECT
public:
    explicit QDynamicWidget(QWidget *parent = 0);
    ~QDynamicWidget();
    static int ResWtID;                                                                 // Счетчик виджетов с кнопками
    int getWtID();                                                                      // Возвращает номер виджета

private:
    int widgetID;                                                                       // Номер виджета с кнопками
};

#endif // QDYNAMICWIDGET_H
