#ifndef INFORMATION_H
#define INFORMATION_H

#include <QObject>
#include <QStringList>
#include <QProcess>

class Information : public QObject
{
    Q_OBJECT
public:
    explicit Information(QObject *parent = 0);
    ~Information();
    QStringList getSysInfo();

signals:

public slots:
};

#endif // INFORMATION_H
