#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
/* Класс реализующий функцию инициализации, чтения и настройки параметров камеры
 * и захвата кадров в родительский QImage объект. Должен исполняться в отдельном
 * потоке в цикле оброботки событий.
 * - Флаг takeshot = true запрашивает обновление кадра в объекте QImage, автома-
 * тически сбрасывается по окончании обновления
 * - Флаг caputure = true разрешает цикл захвата видеопотока.
 * - Флаг ctrenum сообщает о том, что захват был завершен и перед запуском нового
 * цикла захвата, необходимо считать доступные и установить параметры драйвера
 *
 * Список доступных разрешений для USB-камеры ELP iMX
 *
 * 			[0]	"1920x1080"	QString     - 9 сек / bmp-кадр
 *          [1]	"1280x720"	QString     - 5 сек / bmp-кадр
 *			[2]	"800x600"	QString     - 2,5 сек / bmp-кадр
 *			[3]	"640x480"	QString     - 0,6 сек / bmp-кадр
 *			[4]	"640x360"	QString
 *			[5]	"352x288"	QString
 *			[6]	"320x240"	QString
 *			[7]	"1920x1080"	QString
 *
 * TODO: Удалить неиспользуемые переменные и функции (использовались в другом проекте).
 *       Удалить закомментированные функции.
 *       Реализовать возможность кропа квадратного кадра (480x480) из FullHD кадра
 * наиболее быстрым методом с последующей его передачей в QImage.
 *       Реализовать непосредственный вывод буфера камеры в кадровый буфер дисплея (/dev/fb0)
 *       Минимизировать копирование кадровых буферов при передаче в QImage
 */
#include <QStringList>
#include <QString>
#include <QVector>                                                                      // Подключаем QVector для сохранения контролов
#include <QThread>

#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "libv4l2.h"
#include "libv4lconvert.h"
#include "videowidget.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))                                             // Макрос очистке буфера памяти

class CaptureThread : public QThread                                                    // Создаём класс, наследник QThread (поток Qt)
{
public:
    explicit CaptureThread(VideoWidget *parent = 0);
    ~CaptureThread();
    bool capture;                                                                       // Флаг включенного захвата видеопотока
    bool ctrenum;                                                                       // Флаг завершения перечисления контролов открытого устройства
    bool takeshot;                                                                      // Флаг - требование сделать снимок

    VideoWidget                     *videovidget;
    QImage                          cs;                                                 // Объект захвата кадра для сохранения в файл
    struct v4l2_format              fmt;                                                // Буфер, содержащий структуру описывающую видеоформат
    struct v4l2_buffer              buf;                                                // Буфер видеоданных
    struct v4l2_requestbuffers      req;                                                // Запрашиваемые буфера
    enum v4l2_buf_type              type;                                               // тип буфера для v4l2
    fd_set                          fds;                                                // ??
    struct timeval                  tv;                                                 // Структура, содержащая время (сек, микросек) ??
    int                             r, fd;                                              // Целые fd - дескриптор файла,  r - возвращаемые коды ошибок
    unsigned int                    n_buffers;                                          // Количество запрашиваемых буферов (int)
    char                            dev_name[16];                                       // Буфер для указания имени устройства видеозахвата
    __u32                           xres, yres;                                         // Рабочее разрешение камеры
    char                            xrstr[5], yrstr[5];                                 // Символьные переменные для хранения разрешения

// Объявляем структуру буфера 'buffer', состоящую из указателя на начало *start и размера буфера (size_t)
    struct buffer {
            void   *start;
            size_t length;
    };

// Объявляем статическую функцию 'xioctl', которая получает на входе дескриптор файла (int), тип запроса (int), указатель (ссылка) на стуктуру данных
    static void xioctl(int fh, int request, void *arg)
    {
            int r;                                                                      // Объявляем переменную r

            do {                                                                        // В цикле
                    r = v4l2_ioctl(fh, request, arg);                                   // передаём в 'r' результат запроса 'v4l2_ioctl' с параметрами переданными функции
            } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));               // пока получаем в ответ код ошибки (-1) , обусловленный системным прерыванием (errno=4) или повторным запросом (TRY AGAIN)

            if (r == -1) {                                                              // Если функция вернёт код ошибки (-1) при иных обстоятельствах,
                    fprintf(stderr, "error %d, %s\n", errno, strerror(errno));          // Сообщаем о данной ошибке в консоль
                    return;                                                             // Выходим из функции
            }
    }


    void run();                                                                         // Объявляем метод 'run()'

    struct buffer                   *buffers;                                           // Объявляем указатель '*buffers' на структуру типа 'buffer'

// Вводим контролы для управления камерой
    struct v4l2_queryctrl           queryctrl;                                          // структура контрола управления камерой
    struct v4l2_querymenu           querymenu;                                          // структура меню управления камерой
    struct v4l2_control             control;                                            // структура передаваемого контрола управления камерой
    struct v4l2_capability          device_params;                                      // структура для запроса параметров текущего устройства видеозахвата
    struct v4l2_frmsizeenum         frame_fmts;                                         // структура для запроса доступных разрешений видеозахвата
    struct v4l2_fmtdesc             video_fmt;                                          // структура для запроса поддерживаемых видеоформатов
    QVector<v4l2_queryctrl>         camctrl;                                            // Объявляем публичный вектор для хранения всех контролов управления

    void stopUlan();                                                                    // Объявляем метод для остановки захвата
    void startUlan();                                                                   // Объявляем метод для начала захвата

// Добавляем функции перечесления доступных контролов
    void enumerate_menu();                                                              // Перечисляет доступные для камеры параметры съемки
    void enumerate_menu(__u32 id, QStringList &lst);                                    // Возвращает список &lst доступных значений для выбранного праметра
    void setValue(__u32 id, int);                                                       // Устанавливает значение выбранного параметра
    void setDevice(const char *);                                                       // Выбор устройства захвата. На входе символьное имя типа /dev/video0
    void readParameters(QString &s);
    void readResolutions(QStringList &resolutions);                                     // Чтение доступных разрешений камеры
    void setResolution(int);                                                            // Установить требуемое разрешение камеры
};

#endif // CAPTURETHREAD_H
