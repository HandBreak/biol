#include "videowidget.h"
#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent)
{
    setAutoFillBackground(true);
}

void VideoWidget::paintEvent(QPaintEvent *) {                                           // Формируем видеовиджет (по событию отрисовки)
    try{                                                                                // Пробуем
        QPainter painter(this);                                                         // Создаём объект 'painter', наследник 'videowidget'
        painter.setPen(Qt::red);                                                        // Говорим что у нас будет синее перо,
        painter.setFont(QFont("Arial", 30));                                            // а  шрифт установим  'Arial 30'
        painter.drawText(rect(), Qt::AlignCenter, "No Video");                          // и отрисуем в прямоугольной области, по центру, надпись "Qt"

        if(!img.isNull())                                                               // Если объект типа 'QImage' не пустой, тогда
/*          painter.drawImage(QRect(0, 0, 320, 240), img);                              // отрисуем его содержимое в относительных координатах 0,0,
 *                                                                                         шириной 240 x 320 c масштабированием) - для полного кадра (640x480)
 *                                                                                         без нарушения пропорций, но угол кадра
 */
            painter.drawImage(QRect(0, 0, 240, 240), img);                              // отрисуем его содержимое в относительных координатах 0,0,
                                                                                        // шириной 240 x 240 c масштабированием) - для полного кадра (640x480)
                                                                                        // с нарушением пропорций 4:3, но центр кадра
    }catch(...){}                                                                       // перехватим все возможные ошибки
}

VideoWidget::~VideoWidget()
{
    qDebug() << "VideoWidget destroyed!";
}
