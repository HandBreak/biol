#include "tabletwidget.h"

TabletWidget::TabletWidget(QWidget *parent) :
    QWidget(parent)
{
    stLayout = new QStackedLayout;                                                      // Слой для размещения кнопок выбора лунок. Существует все время работы программы
}

TabletWidget::~TabletWidget()
{
    delete stLayout;                                                                    // При завершении программы удаляется слой элементов выбора лунок
    short i,j;
    for (i = 0; i < 12; ++i) {
        for (j = 0; j < 8; ++j) {
            if (h[i][j])
            {
                QObject::disconnect(h[i][j], 0, 0, 0);                                  // Отсоединить все сигналы от массива кнопок
                h[i][j]->deleteLater();                                                 // Вместе с самими элементами, в зависимости от того, сколько было инициализировано
                h[i][j] = NULL;
                qDebug()<<"Destructed h"<<i<<" "<<j;
            }
        }
    }
}
void TabletWidget::setTablet96Holes()
{
    setTabletHoles(12, 8);
}
void TabletWidget::setTablet48Holes()
{
    setTabletHoles(8, 6);
}
void TabletWidget::setTablet24Holes()
{
    setTabletHoles(6, 4);
}
void TabletWidget::setTablet12Holes()
{
    setTabletHoles(4, 3);
}
void TabletWidget::setTabletHoles(short x, short y)
{
    for (int lays = stLayout->count(); lays >= 0 ; lays--)                              // Цикл удаления всех ранее созданных страниц с кнопками выбора лунок, если таковые существуют
    {
        delete stLayout->takeAt(lays -  1);                                             // Удаляем последнюю страницу в массиве
    }
    QGridLayout *layout = NULL;                                                         // Слой кнопок. Матрица создаётся в цикле в зависимости от числа элементов
    QGridLayout *bklayout;                                                              // Слой для размещения виджета выбора лунок
    QWidget *tbWidget;                                                                  // Виджет для размещения слоя с кнопками выбора лунок (разделен с bkWidget для автоцентровки)
    QWidget *bkWidget;                                                                  // Виджет для размещения виджета с кнопками выбора лунок и слоя с кнопками управления
    short i,j;
    for (i = 0; i < x; ++i) {                                                           // Цикл создания массива x*y элементов управления лунками
        for (j = 0; j < y; ++j) {
            if ((i == 0 || i == (x < 8 ? 0 : (x / 2))) && j == 0)                       // Если элементов по оси X более 8, создаём на на одной странице половину элементов, а вторую половину на следующей
            {                                                                           // На кажду страницу создаётся свой набор слоёв и виджетов с элементами выбора лунок
                // layout может оказаться не проинициализированным !!! (поставил NULL в начале)
                layout = new QGridLayout;
                bklayout = new QGridLayout;
                tbWidget = new QWidget;
                bkWidget = new QWidget;
                switch (x) {
                case 12:
                    bkWidget->setFixedSize(240,280);                                    // Размер виджета, на который помещается виджет с ячейками и виджет с элементами управления (снизу)
                    tbWidget->setFixedSize(200,260);                                    // Размер виджета для кнопок планшета 12x8 (размер задаётся для равномерного распределения кнопок по полю)
                    tbWidget->setStyleSheet(" QPushButton  {"
                                            " height: 18px;"
                                            " width:  18px;"
                                            " background-color: green;"
                                            " color: white;"
                                            " border-style: outset;"
                                            " border-width: 2px;"
                                            " border-radius: 16px;"
                                            " border-color: beige;"
                                            " font: bold 9px;"
                                            " min-width: 0em;"
                                            " padding: 6px;} "
                                            " QPushButton:hover  {"
                                            " background-color: rgb(232,95,76); } "
                                            " QPushButton:pressed {"
                                            " background-color: yellow; }"
                                            );
                    break;
                case 8:
                    bkWidget->setFixedSize(240,280);                                    // Размер виджета, на который помещается виджет с ячейками и виджет с элементами управления (снизу)
                    tbWidget->setFixedSize(180,260);                                    // Размер виджета для кнопок планшета 8x6 (размер задаётся для равномерного распределения кнопок по полю)
                    tbWidget->setStyleSheet(" QPushButton  {"
                                            " height: 26px;"
                                            " width:  26px;"
                                            " background-color: green;"
                                            " color: white;"
                                            " border-style: outset;"
                                            " border-width: 2px;"
                                            " border-radius: 21px;"
                                            " border-color: beige;"
                                            " font: bold 12px;"
                                            " min-width: 0em;"
                                            " padding: 6px;} "
                                            " QPushButton:hover  {"
                                            " background-color: rgb(232,95,76); } "
                                            " QPushButton:pressed {"
                                            " background-color: yellow; }"
                                            );
                    break;
                case 6:
                    bkWidget->setFixedSize(240,280);                                    // Размер виджета, на который помещается виджет с ячейками и виджет с элементами управления (снизу)
                    tbWidget->setFixedSize(220,150);                                    // Размер виджета для кнопок планшета 6x4 (размер задаётся для равномерного распределения кнопок по полю)
                    tbWidget->setStyleSheet(" QPushButton  {"
                                            " height: 20px;"
                                            " width:  20px;"
                                            " background-color: green;"
                                            " color: white;"
                                            " border-style: outset;"
                                            " border-width: 2px;"
                                            " border-radius: 18px;"
                                            " border-color: beige;"
                                            " font: bold 10px;"
                                            " min-width: 0em;"
                                            " padding: 6px;} "
                                            " QPushButton:hover  {"
                                            " background-color: rgb(232,95,76); } "
                                            " QPushButton:pressed {"
                                            " background-color: yellow; }"
                                            );
                    break;
                case 4:
                    bkWidget->setFixedSize(240,280);                                    // Размер виджета, на который помещается виджет с ячейками и виджет с элементами управления (снизу)
                    tbWidget->setFixedSize(220,165);                                    // Размер виджета для кнопок планшета 4x3 (размер задаётся для равномерного распределения кнопок по полю)
                    tbWidget->setStyleSheet(" QPushButton  {"
                                            " height: 36px;"
                                            " width:  36px;"
                                            " background-color: green;"
                                            " color: white;"
                                            " border-style: outset;"
                                            " border-width: 2px;"
                                            " border-radius: 26px;"
                                            " border-color: beige;"
                                            " font: bold 14px;"
                                            " min-width: 0em;"
                                            " padding: 6px;} "
                                            " QPushButton:hover  {"
                                            " background-color: rgb(232,95,76); } "
                                            " QPushButton:pressed {"
                                            " background-color: yellow; }"
                                            );
                    break;

                default:
                    break;
                }
                tbWidget->setLayout(layout);                                            // Поместить слой элементов выбора лунок на его виджет
                layout->setMargin(0);                                                   // Толщина сетки
                layout->setSpacing(0);                                                  // и интервалы = 0
                bklayout->addWidget(tbWidget, 1, 1, Qt::AlignCenter);                   // На общий слой добавить виджет элементами выбора лунок. С центрированием
                bkWidget->setLayout(bklayout);                                          // На базовый виджет наложить общий слой
                stLayout->addWidget(bkWidget);                                          // В стек слоёв размещения виджета выбора, добавить новую страницу (может быть несколько) в цикле
            }
            h[i][j] = new QPushButton(QChar(65 + j).toAscii() + QString::number(i + 1));// Формируем массив маркированных элементов выбора лунок
            h[i][j]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);             // Для каждого элемента устанавливаем свойства изменения размера (во избежание изменений под вписанный текст)
            h[i][j]->setFocusPolicy(Qt::NoFocus);                                       // Для каждого элемента отключаем реакцию на фокус (не должны самостоятельно менять цвет при попадании в фокус)
            h[i][j]->setObjectName(QString::number(i) + ";" + QString::number(j));      // Каждому элементу даём имя типа 'x;y' для последующего определения источника сигнала нажатия
            QObject::connect(h[i][j], SIGNAL(clicked(bool)), this, SLOT(clickedButton()));
            layout->addWidget(h[i][j],j,i,Qt::AlignCenter);                             // После соединения сигналов 'clicked' от элементов со слотом, добавляем элемент на слой элементов выбора лунок
        }
    }
    stLayout->setCurrentIndex(0);                                                       // По завершении выбираем активную страницу (Было = 1 !!! 13102019)
}
void  TabletWidget::clickedButton()                                                     // Отработка сигналов от нажатия на элементы управления выбором лунок
{
    QObject* obj=QObject::sender();
    QString str = QString((obj->objectName()));                                         // Получаем имя источника формата 'x;y'
    QStringList list = str.split(";");
    if (list.size() != 2)
        return;
    emit toggleHole(list);                                                              // Если получено корректное имя, отправляем сигналом координаты в виде списка из двух элементов
}                                                                                       // В ином случае не реагируем на нажатие
