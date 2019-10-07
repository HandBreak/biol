#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
/* Класс видеовиджета. Указатель на объект QImage 'img' передаётся в объекту класса
 * CaptureThread для записи в него содержимого захваченного с видеокамеры кадра.
 * События обновления виджета также вызываются из потока CaptureThread
 *
 * TODO: Предусмотреть отображение круговой разметки для калибровки планшета
 */
#include <QWidget>
#include <QPainter>

class VideoWidget : public QWidget
{
Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();
    QImage img;                                                                         // Создаём объект 'img' класса QImage в который будет
                                                                                        // помещаться содержимое захваченного кадра
                                                                                        // (производится в функции capturethread)
protected:
    void paintEvent(QPaintEvent *);
};

#endif // VIDEOWIDGET_H
