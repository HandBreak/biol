#include "arduino.h"
#include <QDebug>

Arduino::Arduino(ActuatorInterface &ai, QObject *parent):
    QObject(parent)
{
    bottomZLimit = Zmin;                                                                // Инициализируем нижний предел для оси Z
    qDebug()<<"Arduino created!";
    actInterface = &ai;                                                                 // Инициализируем указатель адресом драйвера интерфейса tty
}

Arduino::~Arduino()
{
    heatingOff();                                                                       // При завершении работы отключить подогрев
    motorOff();                                                                         // При завершении работы отключить ток двигателей
    coolerOff();                                                                        // При завершении работы отключить вентилятор
    lightOff();                                                                         // При завершении работы отключить подсветку
    qDebug()<<"Arduino destroyed!";
}

void Arduino::sleep(int ms)                                                             // Реализуем функцию паузы в мсек.
{
   static QMutex mutex;
   static QMutexLocker locker(&mutex);
   mutex.tryLock(ms);
}

void Arduino::setBottomLimit(int l)                                                     // Устанавливаем нижний предел каретки для выбранного планшета
{
    bottomZLimit = l;
}

void Arduino::shiftX(int x)
{
    if ( getMoveStatus() == false )                                                     // выполнить только при отсутствии движения любых приводов
    {
        QStringList currentPosition = getPosition();                                    // запрос текущего положения;
        int np = currentPosition.at(X_AXIS).toFloat() * devider + x;
        if (np >= 0 && np < maxXpos)
            setXPosition(np);
    }
}

void Arduino::shiftY(int y)
{
    if ( getMoveStatus() == false )                                                     // выполнить только при отсутствии движения любых приводов
    {
        QStringList currentPosition = getPosition();                                    // запрос текущего положения;
        int np = currentPosition.at(Y_AXIS).toFloat() * devider + y;
        if (np <= maxYpos / 2 && np > defYpos)
            setYPosition(np);
    }
}

void Arduino::shiftZ(int z)
{
    if ( getMoveStatus() == false )                                                     // выполнить только при отсутствии движения любых приводов
    {
        QStringList currentPosition = getPosition();                                    // запрос текущего положения;
        int np = currentPosition.at(Z_AXIS).toFloat() * devider + z;
        if (np <= maxZpos)
            setZPosition(np);
    }
}

void Arduino::sendMoveStatus()
{
    short status = getMoveStatus();
    if (status == 1)
        emit movingInProcess(true);
    if (status == 0)
        emit movingInProcess(false);
    if (status == -1)
        emit executeError(DATA);
}

void Arduino::sendTemperature()
{
    getTemperature();
    if (temperatureList.size()<2)                                                       // При включенном вентиляторе может появляеться третий параметр (7) !!!
    {
        emit executeError(DATA);                                                        // Ошибка чтения данных
        return;
    }
    emit extruderTemperature(temperatureList.at(EXTRUDER).toShort());
    emit tabletTemperature(temperatureList.at(TABLE).toShort());
}

void Arduino::sendStatus()
{
    getEndsStatus();
    if (statusList.size()!=4)
    {
        emit executeError(DATA);                                                        // Ошибка чтения данных
        return;
    }
    emit xEndClosed(statusList.at(X_AXIS).toShort());
    emit yEndClosed(statusList.at(Y_AXIS).toShort());
    emit zEndClosed(statusList.at(Z_AXIS).toShort());
    emit bufferStatus(statusList.at(STATUS));                                           // !!! Разобраться с отсылаемым значением  READY / MOVE !!!
}

void Arduino::sendCurrentPosition()
{
    getPosition();
    if (positionList.size()!=4)
    {
        emit executeError(DATA);                                                        // Ошибка чтнения данных
        return;
    }
    emit axisPosition(positionList.at(X_AXIS).toFloat() * devider, \
                      positionList.at(Y_AXIS).toFloat() * devider, \
                      positionList.at(Z_AXIS).toFloat() * devider  );
}

short Arduino::getMoveStatus()
{
    qDebug()<<"Get current drives status";
    QString moving;
    moving = actInterface->talk("M121");
    if (moving.endsWith("T"))                                                           // Контроллер возвращает код 'T', если хотя бы один из приводов исполняет задание
        return true;
    if (moving.endsWith("F"))
        return false;
    return -1;                                                                          // Вернуть "Ошибка", если контроллер не ответил ожидаемым кодом
}

QStringList Arduino::getLPosition()
{
    return positionList;                                                                // Для "встряхивания" (класс TaskExecutor) возвращает позицию приводов до получения новых координат
}

QStringList Arduino::getPosition()
{
    qDebug()<<"Get Current position command received";
    positionList = actInterface->talk("M114").split(QRegExp("[A-Z: a-z]+"),QString::SkipEmptyParts);
    return positionList;
}

QStringList Arduino::getEndsStatus()
{
    qDebug()<<"Get Status command received";
    statusList = actInterface->talk("M119").split(QRegExp("[\\D]+-max:|( MoveMode:)"),QString::SkipEmptyParts);
    return statusList;
}

QStringList Arduino::getTemperature()
{
    qDebug()<<"Get Temperature command received";
    temperatureList = actInterface->talk("M105").split(QRegExp("(T\\w:)|(/\\d)|[ok\\s]+"),QString::SkipEmptyParts);
    return temperatureList;
}

bool Arduino::setXPosition(int x, USHORT s)
{
    qDebug()<<"Set X-position command received";
    float mm = x / devider;
    bool success = actInterface->talk("G1X" + QString::number(mm)+\
                          "F" + QString::number(s)).contains("ok");
    emit movedX(success);
    return success;
}

bool Arduino::setYPosition(int y, USHORT s)
{
    qDebug()<<"Set Y-position command received";
    float mm = y / devider;
    bool success = actInterface->talk("G1Y" + QString::number(mm)+\
                          "F" + QString::number(s)).contains("ok");
    emit movedY(success);
    return success;
}

bool Arduino::setZPosition(int z, USHORT s)
{
    qDebug()<<"Set Z-position command received";
    if (z < bottomZLimit)
            z = bottomZLimit;                                                           // Ограничим снижение каретки в пределах допустимого для планшета лимита
    float mm = z / devider;
    bool success = actInterface->talk("G1Z" + QString::number(mm)+\
                          "F" + QString::number(s)).contains("ok");
    emit movedZ(success);
    return success;
}

void Arduino::setAPosition(int x, int y, int z, USHORT s)
{
    qDebug()<<"Go to position";
    if (z < bottomZLimit)
            z = bottomZLimit;                                                           // Ограничим снижение каретки в пределах допустимого для планшета лимита
    float xmm = x / devider;
    float ymm = y / devider;
    float zmm = z / devider;
    emit movedA(actInterface->talk("G1X" + QString::number(xmm)+\
                      "Y" + QString::number(ymm)+\
                      "Z" + QString::number(zmm)+\
                      "F" + QString::number(s)).contains("ok"));
}

void Arduino::setAbsoluteCoordinates()
{
    qDebug()<<"Set Absolute Coordinates command received";
    absCrd = true;
    bool success = actInterface->talk("G90").contains("ok");
    if (success)
        emit absoluteCoordinates(absCrd);
    else
        emit executeError(MODE);
}

void Arduino::setRelativeCoordinates()
{
    qDebug()<<"Set Relative Coordinates command received";
    absCrd = false;
    bool success = actInterface->talk("G91").contains("ok");
    if (success)
        emit absoluteCoordinates(absCrd);
    else
        emit executeError(MODE);
}

bool Arduino::setCurrentCoordinates(int x, int y, int z)
{
    qDebug()<<"Set Current position command received";
    float xmm = x / devider;
    float ymm = y / devider;
    float zmm = z / devider;
    return actInterface->talk("G92X" + QString::number(xmm)+\
                   "Y" + QString::number(ymm)+\
                   "Z" + QString::number(zmm)).contains("ok");
}

bool Arduino::homing(short a,  short i)
{
    QString axis;
    int pause;
    short spd;
    short lim;
    short d = 1, m = i;                                                                 // Делитель скорости и контроль множителя
    bool e = true;

    switch (a) {                                                                        // Выбираем ось для которой выполняется хоминг
    case X_AXIS:
        axis = "X";
        spd = speedWhome;
        lim = overWlimits;
        pause = pauseWhome;
        break;
    case Y_AXIS:
        axis = "Y";
        spd = speedDhome;
        lim = overDlimits;
        pause = pauseDhome;
        break;
    case Z_AXIS:
        axis = "Z";
        spd = speedHhome;
        lim = overHlimits;
        pause = pauseHhome;
        break;
    default:
        return false;                                                                   // Запрошена несуществующая ось
    }
    do
    {
        do
        {
            e = e && (actInterface->talk("G1" + axis + QString::number(lim * i)+\
                                   "F" + QString::number(spd / d)).contains("ok"));
            if (i == m)
            {
                do
                {
                    getEndsStatus();                                                    // Запросим положение концевиков.
                    sendMoveStatus();                                                   // Дополняем!!! Запрос статуса движения с отправкой соответствующих сигналов (информируем пользователя о процессе исполнения)
                    if (statusList.size() < 3)                                          // Корректный ответ контроллера должен содержать список для 3х или более приводов
                    {
                        emit executeError(DATA);
                        return e = false;
                    }
                } while (!statusList.at(a).toShort());
                actInterface->talk("M112");
            };
            i = -1 * i;                                                                 // Изменить множитель направления -/+
            d++ ;                                                                       // Увеличить делитель скорости на 1 (до 2)
        } while (d == 2);                                                               // Повторять процедуру пока не пройдет три цикла (второй с делением на 2)
        if (i == m)
        {
            sleep(pause);                                                               // Выжидаем 0,4 секунды
            actInterface->talk("M112");
            d--;                                                                        // Восстановить делитель скорости до 2
        }
    } while (i == m);
    return e;                                                                           // Вернуть true если не возникло ошибок, иначе false
}

bool Arduino::homing()
{
    bool e = true;                                                                      // Контроль исполнения этапов калибровки
    qDebug()<<"Homing command received";
    actInterface->talk("G91");                                                          // Перейти к относительным координатам
    actInterface->talk("M18");                                                          // Снять ток с двигателей.

    e = homing(Z_AXIS, 1) && e;                                                         // Выполнить юстировку оси X (не менять местами - оптимизация!)
    e = homing(X_AXIS, 1) && e;                                                         // Выполнить юстировку оси X (не менять местами - оптимизация!)
    e = homing(Y_AXIS, -1) && e;                                                        // Выполнить юстировку оси Y (не менять местами - оптимизация!)

    actInterface->talk("G90");                                                          // Переход к абсолютным координатам
    setCurrentCoordinates();                                                            // Установить дефолтные значения координат для текущего положения
    sleep(2000);                                                                        // Ждем 2 секунды
    actInterface->talk("M112");                                                         // Экстренный стоп.
    actInterface->talk("M18");                                                          // Снять ток с двигателей
    emit homed(e);                                                                      // Послать сигнал о завершении хоминга (true / false). true, если успех по всем осям
    return e;                                                                           // Вернуть результат выполнения операции (true /false)
}

void Arduino::emergencyStop()
{
    qDebug()<<"Emergency! command received";
    emit stopped(actInterface->talk("M112").contains("ok"));                            // Послать сигнал о аварийной остановке
}

void Arduino::motorOn()
{
    qDebug()<<"Motor ON command received";
    motor = true;
    bool success = actInterface->talk("M17").contains("ok");
    if (success)
        emit motorPower(motor);
    else
        emit executeError(MOTOR);
}

void Arduino::motorOff()
{
    qDebug()<<"Motor OFF command received";
    motor = false;
    bool success = actInterface->talk("M18").contains("ok");
    if (success)
        emit motorPower(motor);
    else
        emit executeError(MOTOR);
}

void Arduino::lightOn()
{
    qDebug()<<"Light ON command received";
    light = true;
    bool success = actInterface->talk("M145").contains("ok");
    if (success)
        emit lightPower(light);
    else
        emit executeError(LIGHT);
}

void Arduino::lightOff()
{
    qDebug()<<"Light OFF command received";
    light = false;
    bool success = actInterface->talk("M144").contains("ok");
    if (success)
        emit lightPower(light);
    else
        emit executeError(LIGHT);
}

void Arduino::coolerOn()
{
    qDebug()<<"Cooler ON command received";
    cool = true;
    bool success = actInterface->talk("M106").contains("ok");
    if (success)
        emit coolerPower(cool);
    else
        emit executeError(COOLER);
}

void Arduino::coolerOff()
{
    qDebug()<<"Cooler OFF command received";
    cool = false;
    bool success = actInterface->talk("M107").contains("ok");
    if (success)
        emit coolerPower(cool);
    else
        emit executeError(COOLER);
}

void Arduino::heatingOff()
{
    qDebug()<<"Heating OFF command received";
    heat = false;
    bool success = actInterface->talk("M104").contains("ok");                           // Выключить нагрев экструдера.
         success = actInterface->talk("M140").contains("ok") && success;                // Выключить нагрев стола (M140 без параметров сбрасывает S->0)
    if (success)
        emit heaterPower(heat);
    else
        emit executeError(HEATER);
}

void Arduino::heating(short t)
{
    qDebug()<<"Heating ON command received";
    heat = true;
    bool success = (actInterface->talk("M140 S" + QString::number(t)).contains("ok"));  // Включить нагрев стола до t-градусов
    if (success)
        emit heaterPower(heat);
    else
        emit executeError(HEATER);
}
