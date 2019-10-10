#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>

#include <task.h>
#include <arduino.h>
#include <actuatorconstants.h>


class TaskExecutor : public QObject
{
    Q_OBJECT
public:
    explicit TaskExecutor(Arduino *aci, QObject *parent = 0);
    bool wait;                                                                          // Флаг приостановки съемки до завершения сохранения кадра в основном потоке
    int holes;                                                                          // Рассчетное число всех снимков лунок в процессе эксперимента (для определения места на диске)

private:
    void getShot(short x,                                                               // Текущий номер лунки по X
                 short y,                                                               // Текущий номер лунки по Y
                 short l,                                                               // Текущий цикл съемки
                 short n,                                                               // Номер снимка в цикле
                 qint64 t);                                                             // Время в секундах от начала эксперимента
    void swap(int &, int &);                                                            // Обмен X<->Y координат для смены порядка прохода лунок
    void swap(short &, short &);                                                        // Обмен X<->Y координат для смены порядка прохода лунок
    bool shakeTablet();                                                                 // Произвести встряхивание планшета
    Task *task;                                                                         // Указатель на объект задания. Инициализируется при создании
    Arduino *arduino;                                                                   // Указатель на поток управления исполнительными механизмами
    USHORT speed;                                                                       // Определяет скорость перемещения приводов

signals:
    void setTermostat(short);                                                           // Сигнал - запрос на установку температуры (получает данные из задания, отправляет в класс TermoStat)
    void takeShot(QStringList);                                                         // Сигнал - запрос снимка с заданным списком параметров для сохранения в метаданных
    void holeName(QString);                                                             // Сигнал - сообщает номер текущей ячейки (в видеовиджет)
    void experimentInProgress(bool);                                                    // Сообщает о процессе выполнения эксперимента (выполняется /нет), испускается по запросу

public slots:
    void changeTablet();                                                                // Позиционирование для смены планшета, получает сигнал из интерфейса (пауза)
    bool startExperiment(Task *t = 0);                                                  // Исполнить задание, описанное в объекте Task, получает сигнал из интерфейса (начало опыта)
};

#endif // TASKEXECUTOR_H
