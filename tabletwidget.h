#ifndef TABLETWIDGET_H
#define TABLETWIDGET_H
/*
 * Класс реализующий функцию генерации виджета выбора лунок с образцами.
 * Виджет формируется из массива размером до 96 элементов QPushButton
 * с измененным стилем.
 *
 * TODO: Заменить массив QPushButton на менее ресурсоёмкое и более эффективное
 *       решение.
 *       Разобраться с вероятной утечкой памяти (многократном создании массивов!
 */
#include <QWidget>
#include <QDebug>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QStackedLayout>

class TabletWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabletWidget(QWidget *parent = 0);
    ~TabletWidget();
    QPushButton *h[12][8];                                                              // Массив указателей на кнопки выбора лунок
    QStackedLayout *stLayout;                                                           // Указатель на слой размещения кнопок
    void setTablet96Holes();                                                            // Формирует массив для планшета типа 12x8
    void setTablet48Holes();                                                            // Формирует массив для планшета типа 8x6
    void setTablet24Holes();                                                            // Формирует массив для планшета типа 6x4
    void setTablet12Holes();                                                            // Формирует массив для планшета типа 4x3

private:
    void setTabletHoles(short, short);                                                  // Формирует массив из произвольного числа элементов

private slots:
    void clickedButton();

signals:
    void toggleHole(QStringList);
};

#endif // TABLETWIDGET_H
