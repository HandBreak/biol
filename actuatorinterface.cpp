#include "actuatorinterface.h"
#include <QDebug>

ActuatorInterface::ActuatorInterface(QObject *parent) : QObject(parent)
{
    QProcess *p = new QProcess();
    p->start("stty -F /dev/ttyS1 -brkint -icrnl -imaxbel \
             -onlcr -icanon -echo speed 115200 \
             line 0 min 0 time 10");
    p->waitForFinished();                                                               // Настройка порта осуществляется вызовом системной команды stty
    qDebug() << p->readAll();

    tty = new QFile(com);
    if (!tty->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Unbuffered ))    // Порт открывается для прямой записи без буферизации. Передача
        qDebug()<<com<<" open error";                                                   // данных осуществляется побайтно, иначе возникают проблемы с чтением буфера

    qDebug()<<"tty created!";
}

ActuatorInterface::~ActuatorInterface()
{
    tty->close();
    delete tty;
    qDebug()<<"tty destroyed!";
}

QString ActuatorInterface::talk(const QString &string)
{
    QMutexLocker locker(&aMutex);
    sendCommand(string.toAscii().data());
    return receiveAnswer();
}

QString ActuatorInterface::talk(const char string[])
{
    QMutexLocker locker(&aMutex);
    sendCommand(string);
    return receiveAnswer();
}

QString ActuatorInterface::receiveAnswer()
{
    QString s = "";
    short i = 64;                                                                       // Максимальная длина считываемой строки ответа контроллера исполнительных устройств
    do
        {
            QByteArray c = tty->read(1);                                                // Побайтное чтение. Буферизированное по неизвестной причине корректно не работает.
            if (c.toHex() == "0a")
                break;
            s = s + c;
        } while (--i != 0);
    if (i)
        return s;
    emit executeError(TIMEOUT);                                                         // В случае длительного отсутствия ответа от контроллера шлём ошибку таймаута ожидания и
    s = "TIMEOUT";                                                                      // и возвращаем соответствующее текстовое сообщение вместо ответа контроллера
    return s;
}

void ActuatorInterface::sendCommand(const char string[])
{
    tty->write(string);                                                                 // Отправляем текстовую строку - команду на исполнение
    tty->write("\r");                                                                   // всегда завершая ее переводом строки
    tty->flush();                                                                       // и сбрасывая буфер. В ином случае ответ может быть задержан на неопределенный срок.
    usleep(50);
}
