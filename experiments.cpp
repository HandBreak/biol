#include "experiments.h"
#include "ui_experiments.h"

#include "QDebug"

Experiments::Experiments(QStackedWidget *parent, Task *t) :

    QStackedWidget(parent),
    ui(new Ui::Experiments)
{
    this->task = t;
    ui->setupUi(this);

    ui->rb96holes->setChecked(true);
    ui->rbXaxis->setChecked(true);
    ui->chkShake->setChecked(false);
    ui->sbShots->setRange(1, 9);
    ui->sbShots->setValue(1);
    ui->sbCycles->setRange(0, 9);
    ui->sbCycles->setValue(1);
    ui->timeEdit->setTime(QTime(0, 0, 0, 0));
    ui->sbTemp->setRange(0, 60);
    ui->sbTemp->setValue(0);
    ui->sbTemp->setDisabled(true);
    ui->chkHeat->setChecked(false);
    ui->chkVisual->setChecked(true);
    ui->chkCloud->setChecked(true);
    ui->chkSelect->setChecked(true);
    ui->chkStartOf->setChecked(false);
    ui->chkStopOn->setChecked(false);
//    ui->sbStartX->setRange(1, 12);
    ui->sbStartX->setMinimum(1);
    ui->sbStartX->setValue(1);
    ui->sbStartX->setDisabled(true);
//    ui->sbStartY->setRange(1, 8);
    ui->sbStartY->setMinimum(1);
    ui->sbStartY->setValue(1);
    ui->sbStartY->setDisabled(true);
//    ui->sbStopX->setRange(1, 12);
    ui->sbStopX->setMinimum(1);
//    ui->sbStopX->setValue(12);
    ui->sbStopX->setDisabled(true);
//    ui->sbStopY->setRange(1, 8);
    ui->sbStopY->setMinimum(1);
//    ui->sbStopY->setValue(8);
    ui->sbStopY->setDisabled(true);


    tabletWidget = new TabletWidget;
    ui->tabletPage->setLayout(tabletWidget->stLayout);
    setCurrentIndex(0);

    onTabletChanged();                  // Настроим планшет по умолчанию

    QObject::connect(ui->pbCancel, SIGNAL(clicked()),this, SLOT(onReturnClicked()));
    QObject::connect(ui->pbStart, SIGNAL(clicked()),this, SLOT(onStartClicked()));
    QObject::connect(ui->pbPause, SIGNAL(clicked()),this, SLOT(onPauseClicked()));
    QObject::connect(ui->pbContinue, SIGNAL(clicked()),this, SLOT(onContinueClicked()));
    QObject::connect(ui->pbLeft, SIGNAL(clicked()),this, SLOT(onLeftClicked()));
    QObject::connect(ui->pbRight, SIGNAL(clicked()),this, SLOT(onRightClicked()));
    QObject::connect(ui->chkHeat, SIGNAL(toggled(bool)), ui->sbTemp, SLOT(setEnabled(bool)));
    QObject::connect(ui->chkStartOf, SIGNAL(toggled(bool)), ui->sbStartX, SLOT(setEnabled(bool)));
    QObject::connect(ui->chkStartOf, SIGNAL(toggled(bool)), ui->sbStartY, SLOT(setEnabled(bool)));
    QObject::connect(ui->chkStopOn, SIGNAL(toggled(bool)), ui->sbStopX, SLOT(setEnabled(bool)));
    QObject::connect(ui->chkStopOn, SIGNAL(toggled(bool)), ui->sbStopY, SLOT(setEnabled(bool)));
    QObject::connect(ui->chkVisual, SIGNAL(toggled(bool)), this, SLOT(onVisualCtlToggled(bool)));
    QObject::connect(ui->chkSelect, SIGNAL(toggled(bool)), this, SLOT(onSelectHoleCtlToggled(bool)));
    QObject::connect(ui->timeEdit, SIGNAL(timeChanged(QTime)),this, SLOT(onTimeChanged()));
    QObject::connect(ui->sbTemp, SIGNAL(valueChanged(int)),this, SLOT(onTemperatureChanged()));
    QObject::connect(ui->sbCycles, SIGNAL(valueChanged(int)), this, SLOT(onCyclesChanged()));
    QObject::connect(ui->rb12holes, SIGNAL(toggled(bool)), this, SLOT(onTabletChanged()));
    QObject::connect(ui->rb24holes, SIGNAL(toggled(bool)), this, SLOT(onTabletChanged()));
    QObject::connect(ui->rb48holes, SIGNAL(toggled(bool)), this, SLOT(onTabletChanged()));
    QObject::connect(ui->rb96holes, SIGNAL(toggled(bool)), this, SLOT(onTabletChanged()));
    QObject::connect(tabletWidget, SIGNAL(toggleHole(QStringList)), this, SLOT(onHoleToggled(QStringList)));

    qDebug()<<"Experiments method object created!";
}

Experiments::~Experiments()
{
    delete ui;
    delete tabletWidget;
    qDebug()<<"Experiments destroyed!";
}
void Experiments::curHoleNumber(QString hole)
{
    short j = hole.at(0).toAscii() - 65;
    short i = hole.mid(1).toUShort() - 1;

// Переключение между отображаемыми страницами кнопок. Код сильно тормозит в момент перерисовки!!!!
    short cp = tabletWidget->stLayout->currentIndex();
    if (tabletWidget->stLayout->count() > 1)
    {
        if (i >= task->tabletColumns / 2)
        {
            if (cp != 1)
                tabletWidget->stLayout->setCurrentIndex(1);
        }
        else
        {
            if (cp != 0)
                tabletWidget->stLayout->setCurrentIndex(0);
        }
    }
//    tabletWidget->h[i][j]->setFocus();
    tabletWidget->h[i][j]->setStyleSheet("background-color: yellow;");
    x = i;
    y = j;
    QTimer::singleShot(200, this, SLOT(restColor()));
}
void Experiments::restColor()
{
    if (task->excludedHole[x][y] == true)
        tabletWidget->h[x][y]->setStyleSheet("background-color: blue;");
    else
        tabletWidget->h[x][y]->setStyleSheet("background-color: green;");
}
void Experiments::onHoleToggled(QStringList list)
{
    short i = list.at(0).toShort();
    short j = list.at(1).toShort();
    task->excludedHole[i][j] = !task->excludedHole[i][j];
    if (task->excludedHole[i][j] == true)
        tabletWidget->h[i][j]->setStyleSheet("background-color: blue;");
    else
        tabletWidget->h[i][j]->setStyleSheet("background-color: green;");
}
void Experiments::onVisualCtlToggled(bool flag)
{
    emit videoControl(flag && !ui->pbStart->isEnabled());                        // вызвать видждет визуального контроля во время опыта
}
void Experiments::onSelectHoleCtlToggled(bool flag)
{
    if (flag && !ui->pbStart->isEnabled())
        setCurrentIndex(1);                                                     // вызвать видждет выбора ячеек во время опыта
}
void Experiments::onCloseVisualCtl()
{
    ui->chkVisual->setChecked(false);
}
void Experiments::onCyclesChanged()
{
    if (ui->sbCycles->value() == 0)
        ui->timeEdit->setDisabled(false);
    else
        ui->timeEdit->setDisabled(true);
}
void Experiments::onTemperatureChanged()
{
    task->temperatureSet = ui->sbTemp->value();
}
void Experiments::onTimeOut()
{

}
void Experiments::onTimeChanged()
{
    if ((ui->timeEdit->time().second() == 0) &&\
        (ui->timeEdit->time().minute() == 0) &&\
        (ui->timeEdit->time().hour() == 0))
        ui->sbCycles->setEnabled(true);
    else
    {
        ui->sbCycles->setDisabled(true);
    }
}
void Experiments::onTabletChanged()
{
    if (ui->rb96holes->isChecked()) {
        task->tabletColumns = 12;
        task->tabletRows = 8;
        selHoles96();
    }
    else if (ui->rb48holes->isChecked()) {
        task->tabletColumns = 8;
        task->tabletRows = 6;
        selHoles48();
    }
    else if (ui->rb24holes->isChecked()) {
        task->tabletColumns = 6;
        task->tabletRows = 4;
        selHoles24();
    }
    else if (ui->rb12holes->isChecked()) {
        task->tabletColumns = 4;
        task->tabletRows = 3;
        selHoles12();
    };
    ui->sbStopX->setValue(task->tabletColumns);
    ui->sbStopY->setValue(task->tabletRows);

    ui->sbStopX->setMaximum(task->tabletColumns);
    ui->sbStopY->setMaximum(task->tabletRows);

    ui->sbStartX->setMaximum(task->tabletColumns);
    ui->sbStartY->setMaximum(task->tabletRows);
    setValues();
}

void Experiments::onReturnClicked()
{
    if (ui->pbCancel->text() == "Лоток")
    {
        qDebug()<<"This is tray!";
        task->openTray = true;
        ui->pbPause->setText(tr("Дальше"));
        ui->pbCancel->setDisabled(true);
        return;
    }
    task->continueFlag = false;
    this->close();                  // Закрываем текущее окно
    emit toMainReturn();            // Вызываем сигнал открытия основного окна
}

void Experiments::onPauseClicked()
{
    qDebug()<<"Pause pressed, call method: task.print!";
    ui->pbPause->setText(tr("Пауза"));
    ui->pbCancel->setDisabled(false);
    task->pauseEnabled = !task->pauseEnabled;                           // Инвертировать флаг включения паузы при каждом вызове функции
    if (task->pauseEnabled)
        ui->pbCancel->setText(tr("Лоток"));
    else
        ui->pbCancel->setText(tr("Отмена"));
}
void Experiments::onLeftClicked()
{
    short pc, cp;
    pc = tabletWidget->stLayout->count();
    if (pc > 1)
    {
        cp = tabletWidget->stLayout->currentIndex();
        if (--cp < 0 )
            cp = pc - 1;
        tabletWidget->stLayout->setCurrentIndex(cp);
    }
}
void Experiments::onRightClicked()
{
    short pc, cp;
    pc = tabletWidget->stLayout->count();
    if (pc > 1)
    {
        cp = tabletWidget->stLayout->currentIndex();
        if (++cp > (pc - 1))
            cp = 0;
        tabletWidget->stLayout->setCurrentIndex(cp);
    }
}

void Experiments::onStartClicked()
{
//      отобразить выбор ячеек перед началом
    ui->pbContinue->setText(tr("Продолжить"));
    task->methodName = "Гемостаз";
    task->methodCode = "GS";
    task->sendToCloud = ui->chkCloud->isChecked();

    task->horizontMovement = ui->rbXaxis->isChecked();
    task->doingShake = ui->chkShake->isChecked();
    task->shotPerCycle = ui->sbShots->value();
    task->numberCycles = ui->sbCycles->value();
    task->experimentTime = ui->timeEdit->time().second() * 1000 \
                         + ui->timeEdit->time().minute() * 60000 \
                         + ui->timeEdit->time().hour() * 3600000;

    if (ui->chkHeat->isChecked())
        task->temperatureSet = ui->sbTemp->value();
    else
        task->temperatureSet = 0;
    setValues();

     if (ui->chkSelect->isChecked())
     {
          setCurrentIndex(1);
          if (tabletWidget->stLayout->count() > 1)
          {
              ui->pbLeft->setHidden(false);
              ui->pbRight->setHidden(false);
          }
          else
          {
              ui->pbLeft->setHidden(true);
              ui->pbRight->setHidden(true);
          }

          short i, j;
          for (i = 0; i < task->tabletColumns; ++i)
          {
              for (j = 0; j < task->tabletRows; ++j){
                  if (task->excludedHole[i][j] == true)
                      tabletWidget->h[i][j]->setStyleSheet("background-color: blue;");
                  else
                      tabletWidget->h[i][j]->setStyleSheet("background-color: green;");
              }
          }
          return;
     }
     setCurrentIndex(0);
     onContinueClicked();
}
void Experiments::onContinueClicked()
{
//    ui->chkSelect->setChecked(false);
//    onStartClicked();

//      else
//          setCurrentIndex(0);

//    onTabletChanged();                                                  // Вызвать настройку планшета по умолчанию

//
    if (ui->pbContinue->text() == "Закрыть")
    {
        setCurrentIndex(0);
        ui->chkSelect->setChecked(false);
        return;
    }
    ui->pbContinue->setText(tr("Закрыть"));
    emit bottomLimit(task->cameraBottomLimit);  // Установить нижнюю границу оси Z для выбранного планшета
    emit calibrate(task);

    if (task->temperatureSet !=0)
    {
        emit setTermostat(task->temperatureSet);
/*        QMessageBox::information(0,tr("Термостабилизация"), \
                                 tr("Задана стабилизация тем-\n"  \
                                    "пературы эксперимента.  \n"  \
                                    "Ожидайте достижения     \n"  \
                                    "требуемого значения.    \n" \
                                    "Нажмите \"Ок\" для немед-\n"
                                    "ленного продолжения"));
*/
/*        QMessageBox *qmsg = new QMessageBox(QMessageBox::Information,        \
                               tr("Термостабилизация"),         \
                               tr("Задана стабилизация тем-\n"  \
                                  "пературы эксперимента.  \n"  \
                                  "Ожидайте достижения     \n"  \
                                  "требуемого значения.    \n"  \
                                  "Нажмите \"Ок\" для немед-\n" \
                                          "ленного продолжения"));
        QObject::connect(this, SLOT(heated()), qmsg, SLOT(close()));
        qmsg->exec();
        QObject::disconnect(this, SLOT(heated()), qmsg, SLOT(close()));
        delete qmsg;
*/
        qmsg.setWindowTitle(tr("Термостабилизация"));
        qmsg.setText(tr("Задана стабилизация тем-\n"  \
                        "пературы эксперимента.  \n"  \
                        "Ожидайте достижения     \n"  \
                        "требуемого значения.    \n"  \
                        "Нажмите \"Ок\" для немед-\n" \
                                "ленного продолжения"));
        qmsg.exec();
    }
    emit letsStart(task);
}
void Experiments::waitEndOfExperiment(bool flag)
{
    if (!flag)
        setCurrentIndex(0);
    ui->pbStart->setDisabled(flag);
    ui->gbTablet->setDisabled(flag);
    ui->gbDirection->setDisabled(flag);
    ui->chkShake->setDisabled(flag);
    ui->chkStartOf->setDisabled(flag);
    ui->chkStopOn->setDisabled(flag);
    ui->sbCycles->setDisabled(flag);
    ui->sbShots->setDisabled(flag);
    emit videoControl(flag && ui->chkVisual->isChecked());                        // вызвать видждет визуального контроля
}
void Experiments::setValues()
{
    task->startHole[0] = ui->sbStartX->value() - 1;
    task->startHole[1] = ui->sbStartY->value() - 1;

    task->endHole[0] = ui->sbStopX->value() - 1;
    task->endHole[1] = ui->sbStopY->value() - 1;
}

void Experiments::selHoles12()
{
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
    tabletWidget->setTablet12Holes();
}

void Experiments::selHoles24()
{
    //    Установка констант для которых отсутствуют контролы управления (НАСТРОЕНО!)
/*  СТАРЫЙ ПРОЗРАЧНЫЙ ВЫСОКИЙ ПЛАНШЕТ
    task->holeFirstXPosition = 24000; //первая колонка
    task->holeFirstYPosition = 33000; //первый столбец
    task->cameraPosition     = 61000; //(75000 - центр)  !!! 61000 Минимально-допустимая высота!
    task->cameraBottomLimit  = 61000;
    task->cameraTopLimit     = 95000;
    task->holeXSpacing       = 19500; //(97500 / 5 интервалов)
    task->holeYSpacing       = 19500; //(58000 / 3 интервала)
    task->shiftCorrection    =     0;
    task->shotInterval       =  1500; //(1,5 сек)
*/

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

//  ЧЕРНЫЙ НИЗКИЙ ПЛАНШЕТ
//    task->holeFirstXPosition = 27000; //первая колонка
//    task->holeFirstYPosition = 32000; //первый столбец
//    task->cameraPosition     = 57000; //(75000 - центр)  !!! 61000 Минимально-допустимая высота!
//    task->cameraBottomLimit  = 61000;
//    task->cameraTopLimit     = 95000;
//    task->holeXSpacing       = 18000; //(90000 / 5 интервалов  - черный планшет)
//    task->holeYSpacing       = 18000; //(54000 / 3 интервала - черный планшет)
//    task->shiftCorrection    =     0;
//    task->shotInterval       =  1000; //(1,5 сек)

    for (short i = 0; i < task->tabletColumns; i++) {

        for (short j = 0; j < task->tabletRows; j++) {
            task->excludedHole[i][j] = true;
        }
    }
    tabletWidget->setTablet24Holes();
}

void Experiments::selHoles48()
{
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
    tabletWidget->setTablet48Holes();
}

void Experiments::selHoles96()
{
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
    tabletWidget->setTablet96Holes();
}
