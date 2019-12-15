#include "methodselector.h"

MethodSelector::MethodSelector(QString &s, Task *t, QObject *parent) : QObject(parent)
{
    if (s.size() > 4 && s.left(2) == "pb") {
        method = s.mid(2,2);
        version = s.right(1);
        this->task = t;
    }
}

MethodSelector::~MethodSelector()
{

}

void MethodSelector::setTaskMode()
{
    task->horizontMovement = true;
    task->continueFlag = false;
    task->pauseEnabled = false;
    task->doingShake = false;
    task->sendToCloud = true;
    task->openTray = false;
    task->shotPerCycle = 1;
    task->numberCycles = 1;
    task->temperatureSet = 0;
    task->experimentTime = 0 * 1000 \
                         + 0 * 60000 \
                         + 0 * 3600000;
    task->startHole[0] = 0;
    task->startHole[1] = 0;

    if (method.size() == 0)
        return;
    task->methodCode = method;
    if (task->methodCode == "GS")
    {
        task->methodName = "Hemostasis";
        task->temperatureSet = 37;
        selHoles96();
        task->experimentTime = 2 * 3600000;                                             // Время эксперимента не более 2 часов
        switch (version.toShort()) {
        case 1:
            task->shotInterval = 15000;                                                 // (15 сек) !!! Только для одной лунки. Нужна возможность изменять время !!!
            break;
        case 2:
            task->shotInterval = 30000;                                                 // (30 сек) !!! Только для одной лунки. Нужна возможность изменять время !!!
            break;
        default:
            task->shotInterval = 1000;                                                  // (1 сек) !!! Только для одной лунки. Нужна возможность изменять время !!!
            break;
        }
    }
    else if (task->methodCode == "CL")
    {
        task->methodName = "Complement";
        task->temperatureSet = 25;
        selHoles96();
        task->doingShake = true;
        task->experimentTime = 2 * 3600000;                                             // Время эксперимента не более 2 часов
        task->shotPerCycle = 2;                                                         // Сделать время между циклами не менее 1 минуты !!!
        task->shotInterval = 1000;                                                      // (1 сек) !!! Нужна возможность изменения. Встряхивать 1 раз, затем 3 сек ожидания
    }
    else if (task->methodCode == "TX")
    {
        task->methodName = "Toxicity";
        task->temperatureSet = 25;        //  Уточнить
        task->doingShake = true;
        task->numberCycles = 5;                                                         // 5-кратный цикл. (всего два цикла с интервалом 2 или 24 часа в зависимости от метода)
        task->shotPerCycle = 2;                                                         // Сделать время между циклами не менее 1 минуты !!!
        task->shotInterval = 1000;                                                      // (1 сек) !!! Нужна возможность изменения. Встряхивать 1 раз, затем 3 сек ожидания
        selHoles96();
        switch (version.toShort()) {
        case 1:
            selHoles24();
            break;
        default:
            break;
        }
    }
}

void MethodSelector::selHoles12()
{
    task->tabletColumns = 4;
    task->tabletRows = 3;
    task->holeFirstXPosition = 24000;
    task->holeFirstYPosition = 33000;
    task->cameraPosition     = 65000; // (75000 - центр)
    task->cameraBottomLimit  = 65000;
    task->holeXSpacing       = 33000; //(99000 / 3)
    task->holeYSpacing       = 32000; //(64000 / 2)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
    task->endHole[0] = 3;                                                               // Надо находить крайние среди выбранных и ограничивать длини цикла !!!
    task->endHole[1] = 2;
}

void MethodSelector::selHoles24()
{
    task->tabletColumns = 6;
    task->tabletRows = 4;
    task->holeFirstXPosition = 26000; //первая колонка
    task->holeFirstYPosition = 34000; //первый столбец
    task->cameraPosition     = 61000; //(75000 - центр)  !!! 61000 Минимально-допустимая высота!
    task->cameraBottomLimit  = 61000;
    task->holeXSpacing       = 19300; //(96500 / 5 интервалов)
    task->holeYSpacing       = 19300; //(57900 / 3 интервала)
    task->shiftCorrection    =     0;
    task->shotInterval       =  1000; //(1,0 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
    task->endHole[0] = 5;                                                               // Надо находить крайние среди выбранных и ограничивать длини цикла !!!
    task->endHole[1] = 3;
}

void MethodSelector::selHoles48()
{
    task->tabletColumns = 8;
    task->tabletRows = 6;
    task->holeFirstXPosition = 24000;
    task->holeFirstYPosition = 33000;
    task->cameraPosition     = 65000; // (75000 - центр)
    task->cameraBottomLimit  = 65000;
    task->holeXSpacing       = 14000; //(98000 / 7)
    task->holeYSpacing       = 12800; //(64000 / 5)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
    task->endHole[0] = 7;                                                               // Надо находить крайние среди выбранных и ограничивать длини цикла !!!
    task->endHole[1] = 5;
}

void MethodSelector::selHoles96()
{
    task->tabletColumns = 12;
    task->tabletRows = 8;
    task->holeFirstXPosition = 23000;
    task->holeFirstYPosition = 36500;
    task->cameraPosition     = 57000; // (75000 - центр)
    task->cameraBottomLimit  = 57000;
    task->holeXSpacing       =  9000; //(99000 / 11)
    task->holeYSpacing       =  9000; //(63000 / 7)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
    task->endHole[0] = 11;                                                               // Надо находить крайние среди выбранных и ограничивать длини цикла !!!
    task->endHole[1] = 7;
}

