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
    if (method.size() == 0)
        return;
    task->methodName = method;
}

void MethodSelector::execute()
{
    emit letsStart(task);
}

void MethodSelector::set()
{

task->methodName = "Гемостаз";
task->methodCode = "GS";
task->sendToCloud = true;

task->horizontMovement = true;
task->doingShake = false;
task->shotPerCycle = 1;
task->numberCycles = 1;
task->experimentTime = 5 * 1000 \
                     + 0 * 60000 \
                     + 0 * 3600000;

task->temperatureSet = 25;

short i, j;
for (i = 0; i < task->tabletColumns; ++i)
{
    for (j = 0; j < task->tabletRows; ++j){
        if (task->excludedHole[i][j] == true);
//            tabletWidget->h[i][j]->setStyleSheet("background-color: blue;");
        else;
//            tabletWidget->h[i][j]->setStyleSheet("background-color: green;");
    }
}
}

void MethodSelector::selHoles12()
{
    task->tabletColumns = 4;
    task->tabletRows = 3;
    //    Установка констант для которых отсутствуют контролы управления
    task->holeFirstXPosition = 24000;
    task->holeFirstYPosition = 33000;
    task->cameraPosition     = 65000; // (75000 - центр)
    task->cameraBottomLimit  = 65000;
//    task->cameraTopLimit     = 95000;
    task->holeXSpacing       = 33000; //(99000 / 3)
    task->holeYSpacing       = 32000; //(64000 / 2)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
//    tabletWidget->setTablet12Holes();
}

void MethodSelector::selHoles24()
{
    task->tabletColumns = 6;
    task->tabletRows = 4;
//  НОВЫЙ ПРОЗРАЧНЫЙ ВЫСОКИЙ ПЛАНШЕТ
    task->holeFirstXPosition = 26000; //первая колонка
    task->holeFirstYPosition = 34000; //первый столбец
    task->cameraPosition     = 61000; //(75000 - центр)  !!! 61000 Минимально-допустимая высота!
    task->cameraBottomLimit  = 61000;
//    task->cameraTopLimit     = 95000;
    task->holeXSpacing       = 19300; //(96500 / 5 интервалов)
    task->holeYSpacing       = 19300; //(57900 / 3 интервала)
    task->shiftCorrection    =     0;
    task->shotInterval       =  1000; //(1,0 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
//    tabletWidget->setTablet24Holes();
}

void MethodSelector::selHoles48()
{
    task->tabletColumns = 8;
    task->tabletRows = 6;
    //    Установка констант для которых отсутствуют контролы управления
    task->holeFirstXPosition = 24000;
    task->holeFirstYPosition = 33000;
    task->cameraPosition     = 65000; // (75000 - центр)
    task->cameraBottomLimit  = 65000;
//    task->cameraTopLimit     = 95000;
    task->holeXSpacing       = 14000; //(98000 / 7)
    task->holeYSpacing       = 12800; //(64000 / 5)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
//    tabletWidget->setTablet48Holes();
}

void MethodSelector::selHoles96()
{
    task->tabletColumns = 12;
    task->tabletRows = 8;
    //    Установка констант для которых отсутствуют контролы управления (НАСТРОЕНО!)
    task->holeFirstXPosition = 23000;
    task->holeFirstYPosition = 36500;
    task->cameraPosition     = 57000; // (75000 - центр)
    task->cameraBottomLimit  = 57000;
//    task->cameraTopLimit     = 95000;
    task->holeXSpacing       =  9000; //(99000 / 11)
    task->holeYSpacing       =  9000; //(63000 / 7)
    task->shiftCorrection    =     0;
    task->shotInterval       =   300; //(0,3 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
//    tabletWidget->setTablet96Holes();
}

