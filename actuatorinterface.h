#ifndef ACTUATORINTERFACE_H
#define ACTUATORINTERFACE_H
/*
 * Класс реализующий функцию приёма/передачи команд для контроллера исполнительных механизмов
 * через коммуникационный порт. Имя порта определяется в actuatorconstants.h как переменная
 * const QString com = "/dev/ttyS1"
 * Класс принимает текстовую строку как массив char, либо адрес QString и возвращает QString,
 * Должен исполняться в отдельном от интерфейса потоке. Операции Запрос/ответ атомарны.
 * Параметры порта жестко определены в конструкторе класса и не предусматривают изменений
 *
 * TODO: Вынести настройку параметров коммуникационного порта, включая инициализацию во внешний
 * модуль и предусмотреть изменение и сохранение параметров из интерфейса пользователя или через
 * файлы инициализации.
 */
#include <QObject>
#include <QProcess>
#include <QByteArray>
#include <QMutexLocker>
#include <QMutex>
#include <QString>
#include <QFile>

#include <unistd.h>
#include <actuatorconstants.h>


class ActuatorInterface : public QObject
{
    Q_OBJECT
public:
    explicit ActuatorInterface(QObject *parent = 0);
    ~ActuatorInterface();
    QString talk(const char[]);                                                         // Принимает буфер с текстовой командой для Actuator (например  'M124'), возвращает QString с ответом от контроллера
    QString talk(const QString &);                                                      // Принимает адрес QString строки с текстовой командой для Actuator (например  'M124'), возвращает QString с ответом от контроллера

private:
    void sendCommand(const char[]);
    QString receiveAnswer();                                                            // Вызывается следом за отправкой команды в порт для чтения ответа в QString
    QMutex aMutex;
    QFile *tty;                                                                         // Указатель на файл последовательного порта

signals:
    void executeError(int);                                                             // Сигнал о возникновении ошибки исполнения команды, излучает код ошибки, описанный в actuatorconstatns.h (здесь только TIMEOUT)
};

#endif // ACTUATORINTERFACE_H
