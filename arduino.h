#ifndef ARDUINO_H
#define ARDUINO_H
/*
 * Класс реализующий функции управления исполнительными механизмами и преобразованием ответных сообщений
 * включает в себя функции управления приводами, подсветкой, вентиляторами, нагревателями, а так же чтения
 * их состояний, включая температуру нагревательного элемента и исследуемых образцов. Точность измерения
 * температуры ограничена 1гр.цельсия свойствами микропрограммы контроллера исп.механизмов.
 * Кроме того в классе реализована функция калибровки (хоминга) исполнительных механизмов.
 * Класс должен исполняться в независимом от интерфейса потоке.
 *
 * TODO: Удалить неиспользуемые функции, переменные, сигналы и слоты
 */
#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QMutex>
#include <QThread>

#include <actuatorinterface.h>
#include <actuatorconstants.h>

class Arduino : public QObject
{
    Q_OBJECT

public:
    explicit Arduino(ActuatorInterface &ai, QObject *parent = 0);                       // При создании должен быть проинициализирован адресом объекта приёмо/передатчика
    ~Arduino();
    void setAbsoluteCoordinates();                                                      // Установить режим работы в абсолютных координатах
    void setRelativeCoordinates();                                                      // Установить режим работы в относительных координатах
    bool setCurrentCoordinates(int = defXpos, int = defYpos, int = defZpos);            // Установить значения текущих позиций
    bool setXPosition(int xPosition, USHORT = defspeed);                                // Переместить каретку в положение (x) по горизонтали, с указанной или дефолтной скоростью
    bool setYPosition(int yPosition, USHORT = defspeed);                                // Переместить стол в положение (y) по горизонтали, с указанной или дефолтной скоростью
    bool setZPosition(int zPosition, USHORT = defspeed);                                // Переместить каретку в положение (z) по вертикали, с указанной или дефолтной скоростью
    void sleep(int);                                                                    // Реализует функцию паузы (в миллисекундах) через Mutex, используется в том числе внешними классами
    short getMoveStatus();                                                              // В зависимости от состояния исполнительных устройств (движется/нет) возвращает (true/false) или -1 в случае ошибки
    QStringList getPosition();                                                          // Возвращает список текущих координат всех приводов (не фактических, а результирующих) (без проверки)
    QStringList getLPosition();                                                         // Возвращает список последних полученных координат всех приводов без запроса к контроллеру
    QStringList getEndsStatus();                                                        // Возвращает список текущих состояний концевиков для всех (4-х приводов)  (без проверки)
    QStringList getTemperature();                                                       // Возвращает список текущих температур двух датчиков (без проверки)

private:
    ActuatorInterface *actInterface;                                                    // Указатель на объект приемо/передатчика контроллера приводов
    QStringList statusList;                                                             // Хранит список последних полученных состояний всех концевиков (4 параметра)
    QStringList positionList;                                                           // Хранит список последних полученных координат всех приводов   (4 параметра)
    QStringList temperatureList;                                                        // Хранит список последних измеренных температур (2 и в некоторых случаях более! параметра)
    bool homing(short axis, short direction);                                           // Осуществляет функцию "хоминга" для указанной оси
    bool motor;                                                                         // Хранит текущее состояние блокировки ШД (вкл/выкл)            ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    bool light;                                                                         // Хранит текущее состояние подсветки (вкл/выкл)                ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    bool cool;                                                                          // Хранит текущее состояние вентилятора (вкл/выкл)              ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    bool heat;                                                                          // Хранит текущее состояние нагревателя (вкл/выкл)              ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    bool absCrd;                                                                        // Хранит режим абсолютных/относительных координат (вкл/выкл)   ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    int bottomZLimit;                                                                   // Хранит нижнюю границу допустимого вертикального перемещения. Устанавливается из параметров задания (планшета)

public slots:
    void sendStatus();                                                                  // Запросить статус концевиков и буфера. В ответ шлёт 4 сигнала, либо сигнал с кодом ошибки (cmdErr)
    void sendMoveStatus();                                                              // Запросить статус приводов. В ответ шлёт 1 сигнал (false/true),либо сигнал с кодлм ошибки (cndErr)
    void sendTemperature();                                                             // Запросить замер темератур стола и экструдера - в ответ будут отосланы tabTemp и extTemp или cmdErr
    void sendCurrentPosition();                                                         // Запросить текущие координаты. В ответ посылает сигнал 'axisPosition' со списком позиций приводов или сигнал с cmdErr
    void emergencyStop();                                                               // Экстренная остановка. Немедленно останавливает приводы, в ответ шлёт сигнал 'stopped' с true, если команда выполнена
    void motorOn();                                                                     // Включить моторы.   В ответ шлёт сигнал 'motorPower' с true, если включены. В случае ошибки шлёт сигнал с cmdErr
    void motorOff();                                                                    // Выключить моторы.  В ответ шлёт сигнал 'motorPower' с false, если выключены. В случае ошибки шлёт сигнал с cmdErr
    void lightOn();                                                                     // Включить свет.  В ответ шлёт сигнал 'lightPower' с true, если включен. В случае ошибки шлёт сигнал с cmdErr
    void lightOff();                                                                    // Выключить свет. В ответ шлёт сигнал 'lightPower' с false, если выключен. В случае ошибки шлёт сигнал с cmdErr
    void coolerOn();                                                                    // Включить вентилятор.  В ответ шлёт сигнал 'coolerPower' с true, если включен. В случае ошибки шлёт сигнал с cmdErr
    void coolerOff();                                                                   // Выключить вентилятор. В ответ шлёт сигнал 'coolerPower' с false, если выключен. В случае ошибки шлёт сигнал с cmdErr
    void heatingOff();                                                                  // Выключить подогрев.  В ответ шлёт сигнал 'heaterPower' с false, если выключен. В случае ошибки шлёт сигнал с cmdErr
    void heating(short temperature = 0);                                                // Установить температуру нагревателя (по умолчанию не нагревать). В ответ шлёт сигнал 'heaterPower' с true, если включен.
    void shiftX(int);                                                                   // Сместить по X +/- мкм. В ответ шлёт сигнал 'movedX' с true, если выполнено. Игнорируется, если была получена в движении  ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    void shiftY(int);                                                                   // Сместить по Y +/- мкм. В ответ шлёт сигнал 'movedY' с true, если выполнено. Игнорируется, если была получена в движении  ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    void shiftZ(int);                                                                   // Сместить по Z +/- мкм. В ответ шлёт сигнал 'movedZ' с true, если выполнено. Игнорируется, если была получена в движении  ПО ФАКТУ НЕ ИСПОЛЬУЕТСЯ, МОЖНО УДАЛИТЬ !
    void setAPosition(int xPosition,\
                      int yPosition,\
                      int zPosition = defZpos,\
                      USHORT speed = defspeed);                                         // Установить одновременно по всем осям. В ответ шлёт сигнал 'movedA' с true, если выполнено. Отрабатывается всегда.
    void setBottomLimit(int);                                                           // Установка нижней границы для оси Z в зависимости от выбранного планшета. Влияет на setAPosition, shiftZ и setZPosition
    bool homing();                                                                      // Выполняет калибровку приводов по трем осям. В ответ шлёт и возвращает 'homed' с true или false при наличии ошибки

signals:
    void bufferStatus(QString);                                                         // Сообщение о состоянии буфера "MoveMode:" Отсылается на запрос 'sendStatus'. Содержит строку ответа 'READY/MOVE'
    void axisPosition(int, int, int);                                                   // Сообщает текущие позиции по трем осям в ответ на запрос 'sendCurrentPosition'
    void absoluteCoordinates(bool);                                                     // Сигнал о переключении относительных / абсолютных координат. Отсылается в ответ на 'setAbsoluteCoordinates'
    void tabletTemperature(short);                                                      // Сообщает о температуре стола в формате в ответ на запрос readTemp()
    void extruderTemperature(short);                                                    // Сообщает о температуре экструдера в формате в ответ на запрос readTemp()
    void movingInProcess(bool);                                                         // Сообщает о процессе выполнения перемещения (выполняется / нет) в ответ на запрос 'sendMoveStatus'. Либо шлёт ошибку cmdErr

    void executeError(int);                                                             // Сигнал о возникновении ошибки исполнения команды. Отсылается при возникновении любых ошибок, содержит код ошибки
    void movedX(bool);                                                                  // Сигнал о успешном или нет позиционировании по X. Отсылается в ответ на 'setXPosition'
    void movedY(bool);                                                                  // Сигнал о успешном или нет позиционировании по Y. Отсылается в ответ на 'setYPosition'
    void movedZ(bool);                                                                  // Сигнал о успешном или нет позиционировании по Z. Отсылается в ответ на 'setZPosition'
    void movedA(bool);                                                                  // Сигнал о успешном или нет позиционировании по All осям. Отсылается в ответ на 'setAPosition'
    void xEndClosed(bool);                                                              // Сообщает о состоянии концевика X. Отсылается в ответ на 'sendStatus'
    void yEndClosed(bool);                                                              // Сообщает о состоянии концевика Y. Отсылается в ответ на 'sendStatus'
    void zEndClosed(bool);                                                              // Сообщает о состоянии концевика Z. Отсылается в ответ на 'sendStatus'
    void homed(bool);                                                                   // Сигнал о завершении хоминга.
    void stopped(bool);                                                                 // Сигнал о выполнении аварийного стопа
    void motorPower(bool);                                                              // Сигнал о смене состояния мотора
    void lightPower(bool);                                                              // Сигнал о смене состояния освещения
    void coolerPower(bool);                                                             // Сигнал о смене состояния вентилятора
    void heaterPower(bool);                                                             // Сигнал о смене состояние подогревателя
};

#endif // ARDUINO_H
