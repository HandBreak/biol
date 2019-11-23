#include "information.h"

Information::Information(QObject *parent) : QObject(parent)
{

}

Information::~Information()
{

}

QStringList Information::getSysInfo()
{
    QStringList si;
    for (short i=0; i<6; i++)
    {
        si.append("null");
    }

    QProcess *p = new QProcess();
    p->start("uname -r -n -s -v");                                                      // Получаем название, версию дату сборки ядра
    p->waitForFinished();
    si.replace(2, p->readAll().simplified());
    p->start("/usr/bin/v4l2-ctl --list-devices");
    p->waitForFinished();
    si.replace(3, p->readAll().simplified());
//    p->start("cat /proc/cpuinfo");
//    p->waitForFinished();
//    si.replace(4, p->readAll());
    p->start("cat /proc/fb");
    p->waitForFinished();
    si.replace(5, p->readAll().simplified());
    return si;
}
