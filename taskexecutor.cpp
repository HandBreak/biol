#include "taskexecutor.h"
#include <QDebug>

TaskExecutor::TaskExecutor(Arduino *aci, QObject *parent) : QObject(parent)
{
    arduino = aci;                                                                      // Инициализируем указатель адресом обработчика команд исполнительных механизмов (Класс Arduino)ы
}

bool TaskExecutor::startExperiment(Task *t)
{
    this->task = t;
    qDebug()<<"Experiment started!";
    speed = 3000;
    task->openTray = false;
    task->continueFlag = true;
    task->pauseEnabled = false;

    short l  = 0;                                                                       // Счетчик полных циклов
    int   x  = task->holeFirstXPosition;
    short x1 = task->startHole[0];
    int   x2 = task->holeXSpacing;
    short x3 = task->endHole[0];
    int   y  = task->holeFirstYPosition;
    short y1 = task->startHole[1];
    int   y2 = task->holeYSpacing * -1;                                                 // Сдвиг по Y в противоположную сторону
    short y3 = task->endHole[1];

    // Блок предварительного подсчета максимального числа снимков в эксперименте для вычисления занимаемого места на диске
    holes = 0;

    int h = 0;
    for (short j = y1; j <= y3; j++)                                                    // Считаем число планируемых снимков лунок
    {
        for (short i = x1; i <= x3; i++)
            h = h + !task->excludedHole[i][j];
    }
    h = h * task->shotPerCycle;                                                         // Помножим на количество кадров в цикле
    h = h * task->numberCycles;                                                         // Помножим на количество циклов

    if ((task->numberCycles <= 1) && (task->experimentTime > 0))
    {
        holes = task->experimentTime /                                                  // Расчет предельного числа снимков лунок по времени проведения эксперимента
               (task->shotInterval + 300 +                                              // время в миллисекундах на съёмку одной лунки (300мс = t записи файла)
                task->doingShake * shakePause * 2 * shakeCycles +                       // Добавить время на встряхивание (мсек), если включено
                task->holeXSpacing / (eXspeed / 60));                                   // время в миллисекундах на проход между ближайшими лунками
    }
    else
    {
        holes = h;

    }
    if (!h)
    {
        task->continueFlag = false;
        emit setTermostat(0);
        emit experimentInProgress(false);
        return false;
    }
    // Конец блока предварительной оценки максимального числа снимков

    if (!(task->horizontMovement)) {
        swap(x, y);
        swap(x1, y1);
        swap(x2, y2);
        swap(x3, y3);
    }
    emit experimentInProgress(true);
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    qint64 endTime = startTime + task->experimentTime;
    for (short loops = 1; loops <= task->numberCycles; loops++)                         // Количество прогонов
    {
        arduino->setAbsoluteCoordinates();                                              // Перейти к абсолютным координатам
        if (loops == 1)
        {
            arduino->setAPosition(task->holeFirstXPosition,\
                                  task->holeFirstYPosition,\
                                  task->cameraPosition,\
                                  speed);
        }
        else
            arduino->setZPosition(task->cameraPosition);                                // Альтернатива без позиционирования в A0

        while (arduino->getMoveStatus() == 1)                                           // Ждем выполнения
            arduino->sleep(20);                                                         // Опрос каждые 20мс  ДОБАВЛЕНО 050419
        for (short j = y1; \
             j <= y3; \
             j++)
        {
            bool e = true;
            if (task->horizontMovement)
            {
                for (short i = x1; \
                     i <= x3; \
                     i++)
                {
                    e &= task->excludedHole[i][j];
                }
                if (e)
                    continue;
                arduino->setYPosition(y + y2 * j, eYspeed);
            }
            else
            {
                for (short i = x1; \
                     i <= x3; \
                     i++)
                {
                    e &= task->excludedHole[j][i];
                }
                if (e)
                    continue;
                arduino->setXPosition(y + y2 * j, eXspeed);
            }
            while (arduino->getMoveStatus() == 1)                                       // Ждем выполнения
                arduino->sleep(20);                                                     // Опрос каждые 20мс  ДОБАВЛЕНО 050419
            for (short i = x1; \
                 i <= x3; \
                 i++)
            {
                if (task->horizontMovement)
                {
                    if (task->excludedHole[i][j])
                        continue;
                    arduino->setXPosition(x + x2 * i, eXspeed);
                }
                else
                {
                    if (task->excludedHole[j][i])
                        continue;
                    arduino->setYPosition(x + x2 * i, eYspeed);
                }
                while (arduino->getMoveStatus() == 1)                                   // Ждем завершения перемещения
                    arduino->sleep(20);                                                 // Опрос каждые 20мс  ДОБАВЛЕНО 050419

                arduino->sendTemperature();                                             // Измерить и отправить текущие температуры
                arduino->sleep(20);
                short shots = task->shotPerCycle;
                if (shots != 0 && task->doingShake == true) {                           // Если делаем снимок и включено встряхивание
                    shakeTablet();                                                      // Встряхнем планшет
                    arduino->sleep(task->delayAfterShake);                              // Задержка после встряхивания
                }
                arduino->lightOn();                                                     // Включить подсветку
                arduino->sleep(100);
    // РЕШИТЬ ВОПРОС ДВОЙНОГО ЗАМЕРА ТЕМПЕРАТУРЫ (повторный для извлечения параметров съемки)
                while (shots != 0) {
                    emit holeName(QChar(65 + j).toAscii() + QString::number(i + 1));        // Послать номер текущей ячейки
                    if (task->horizontMovement)
                        getShot(i, j, l,\
                                task->shotPerCycle - shots,\
                                QDateTime::currentMSecsSinceEpoch() - startTime);       // Делаем снимок
                    else
                        getShot(j, i, l,\
                                task->shotPerCycle - shots,\
                                QDateTime::currentMSecsSinceEpoch() - startTime);       // Делаем снимок// Делаем снимок
                    shots--;                                                            // Уменьшаем счетчик снимков на единицу
                    arduino->sleep(task->shotInterval);
                }
                arduino->sleep(100);                                                    // Задержка на 0,1 сек для завершения съемки кадра камерой прежде чем гасить свет
    // РЕШИТЬ ВОПРОС С ЗАДЕРЖКОЙ, ВЕРОЯТНО НУЖНА ПРОВЕРКА ФЛАГА takeshot
                arduino->lightOff();                                                    // Отключим подстветку для выбора следующей лунки
                if (task->pauseEnabled == true)
                {
                    QStringList currentPosition = arduino->getPosition();               // запрос текущего положения;
                    do
                    {
                        arduino->sleep(250);                                            // Приостановить поток на 0,2 сек
                        if (task->openTray == true)
                        {
                            task->openTray = false;                                     // сбросить требование открытия лотка
                            arduino->lightOff();                                        // Отключить подсветку образцов на время выдвижения лотка 20.06.2019
                            changeTablet();                                             // выдвинуть лоток
                        }
                        arduino->lightOn();                                             // Включить подсветку образцов на время паузы 20.06.2019
                    } while (task->pauseEnabled == true \
                          && task->continueFlag == true);

                    arduino->lightOff();                                                // Отключить подсветку образцов при снятии с паузы 20.06.2019
                    arduino->setAPosition(currentPosition.at(X_AXIS).toFloat() * devider,\
                                          currentPosition.at(Y_AXIS).toFloat() * devider,\
                                          currentPosition.at(Z_AXIS).toFloat() * devider,\
                                          speed);
                }
                if (!task->continueFlag)
                {
                    i = x3 + 1;
                    j = y3 + 1;
                    loops = task->numberCycles + 2;                                     // Увеличим на 2, а не на 1 чтобы даже уменьшение loops на 1 из-за неистекшего времени не мешало прерыванию цикла
                }
            }
        }
        l++;                                                                            // Увеличим количество полных циклов на единицу
       if (endTime >= QDateTime::currentMSecsSinceEpoch())
           --loops;
       if (loops <= task->numberCycles)
           arduino->sleep(task->delayAfterLoop);                                        // Ожидание, если не последний цикл
    }
    changeTablet();
    task->continueFlag = false;
    emit setTermostat(0);                                                               // Выключить термостабилизацию
    arduino->heatingOff();
    arduino->lightOff();
    emit experimentInProgress(false);
    qDebug()<<"Experiment ended!";
    return true;
}

void TaskExecutor::changeTablet()
{
    arduino->setZPosition(150000, 2000);
    arduino->setYPosition(75000, 10000);
    arduino->setXPosition(0, 15000);
}

void TaskExecutor::getShot(short x, short y, short l, short n, qint64 t)
{
    wait = true;                                                                        // Поставим на паузу до сохранения кадра (сбрасывается из Main)
    short time = t / 1000;                                                              // Переведем от начала эксперимента время в секунды
    short tt = task->tabletColumns * task->tabletRows;
    QChar c = (char)(((int)'A') + y);
    QStringList tagInfo;
    tagInfo.append("Method," + task->methodCode);                                       // Передадим используемый метод:  "Method,GS0"
    tagInfo.append("TabletType," + QString().setNum(tt));                               // Передадим количество лунок:    "TabletType,96"
    tagInfo.append("CurrentTime," + QDateTime::currentDateTime().toString());           // Передадим текущее время снимка:"CurrentTime,030119.23:14:59"
                                                                                        //    tagInfo.append("HoleAddress," + QString(c) + QString().setNum(++x));
                                                                                        //    tagInfo.append("HoleAddress," + QString(c) + QString("%1").arg(++x, 2, 10, QChar('0')));
    tagInfo.append("HoleAddress," + QString(c) + QString("0%1").arg(++x).right(2));     // Передадим адрес текущей лунки: "HoleAddress,A3"
    tagInfo.append("LoopNumber," + QString().setNum(++l));                              // Передадим номер цикла
    tagInfo.append("ShotNumber," + QString().setNum(++n));                              // Передадим номер снимка в цикле
    tagInfo.append("Temperature," + arduino->getTemperature().at(EXTRUDER).toAscii());  // Передадим текущую температуру: "Temperature,37"
    tagInfo.append("TimeFromStart," + QString().setNum(time));                          // Передадим время в секундах от начала эксперимента
    emit takeShot(tagInfo);
    do
    {
        arduino->sleep(10);
    }   while(wait);                                                                    // Ждём сброс флага 'wait' из Main. Ввести TIMEOUT!!!!
}

bool TaskExecutor::shakeTablet()
{
    bool e = true;
    if (arduino->getPosition().size()!=4)
        return e = false;
    int y = arduino->getLPosition().at(Y_AXIS).toFloat() * devider;
    int shift = shakeShift;
    if ((y + shakeShift) > maxYpos)
        shift = -1 * shakeShift;
    e = arduino->setYPosition(y + shift*2, 5000) && e;
    for (short i = 0; i < shakeCycles; i++)
    {
        e = arduino->setYPosition(y + shift, 5000) && e;
        arduino->sleep(shakePause);
        arduino->emergencyStop();
        e = arduino->setYPosition(y - shift, 5000) && e;
        arduino->sleep(shakePause);
        arduino->emergencyStop();
    }
    e = arduino->setYPosition(y, 5000) && e;
    return e;
}

void TaskExecutor::swap(int &a, int &b)
{
    int r1 = a, r2 = b;
    a = r2;
    b = r1;
}

void TaskExecutor::swap(short &a, short &b)
{
    short r1 = a, r2 = b;
    a = r2;
    b = r1;
}
