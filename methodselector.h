#ifndef METHODSELECTOR_H
#define METHODSELECTOR_H

#include <QObject>
#include <QString>
#include <task.h>

class MethodSelector : public QObject
{
    Q_OBJECT
public:
    explicit MethodSelector(QString  &s, Task *t = 0, QObject *parent = 0);             // Формат имени метода pbXXmethodN, где XX - метод, N - версия
    ~MethodSelector();
    void setTaskMode();
    void execute();
    QString getMethodName() {return (method + version);}
    QString getMethodVersion() {return version;}

private:
    QString method;
    QString version;
    Task *task;
    void selHoles12();
    void selHoles24();
    void selHoles48();
    void selHoles96();
    void set();

signals:
    void letsStart(Task *task);
    void calibrate(Task *task);

public slots:
};

#endif // METHODSELECTOR_H
