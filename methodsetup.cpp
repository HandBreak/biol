#include "methodsetup.h"
#include "ui_methodsetup.h"

MethodSetup::MethodSetup(QWidget *parent, Task *t) :
    QStackedWidget(parent),
    ui(new Ui::MethodSetup)
{
    this->task = t;
    ui->setupUi(this);
    ui->sbInterval->setRange(0,60);
    ui->sbTermostat->setRange(0,50);
    startPrepare();
    tabletWidget = new TabletWidget;
    ui->pgHoleSelector->setLayout(tabletWidget->stLayout);

    setCurrentIndex(0);

    QObject::connect(ui->pbLeft, SIGNAL(clicked()),this, SLOT(onLeftClicked()));
    QObject::connect(ui->pbRight, SIGNAL(clicked()),this, SLOT(onRightClicked()));
    QObject::connect(ui->pbNext, SIGNAL(clicked()),this ,SLOT(onNextClicked()));
    QObject::connect(ui->pbContinue, SIGNAL(clicked()),this, SLOT(onContinueClicked()));
    QObject::connect(ui->pbBack, SIGNAL(clicked()), this, SLOT(onBackClicked()));
    QObject::connect(ui->pbStart, SIGNAL(clicked()), this, SLOT(onStartClicked()));
    QObject::connect(ui->pbEject, SIGNAL(clicked()), this, SLOT(onEjectClicked()));
    QObject::connect(ui->pbControl, SIGNAL(clicked()), this, SLOT(onControlClicked()));
    QObject::connect(ui->cbSaveToCloud, SIGNAL(clicked(bool)),this, SLOT(changedCtrl()));
    QObject::connect(ui->sbInterval, SIGNAL(editingFinished()),this, SLOT(changedCtrl()));
    QObject::connect(ui->sbTermostat, SIGNAL(editingFinished()),this, SLOT(changedCtrl()));
    QObject::connect(ui->pbCam, SIGNAL(clicked()),this, SLOT(onCamClicked()));
    QObject::connect(tabletWidget, SIGNAL(toggleHole(QStringList)), this, SLOT(onHoleToggled(QStringList)));

    qDebug()<<"Experiments MethodSetup object created!";
}

MethodSetup::~MethodSetup()
{
    delete ui;
    delete tabletWidget;
    qDebug()<<"Experiments MethodSetup object destroyed!";
}

void MethodSetup::curHoleNumber(QString hole)
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
    tabletWidget->h[i][j]->setStyleSheet("background-color: yellow;");
    x = i;
    y = j;
    QTimer::singleShot(200, this, SLOT(restColor()));
}

void MethodSetup::restColor()
{
    if (task->excludedHole[x][y] == true)
        tabletWidget->h[x][y]->setStyleSheet("background-color: blue;");
    else
        tabletWidget->h[x][y]->setStyleSheet("background-color: green;");
}

void MethodSetup::onHoleToggled(QStringList list)
{
    short i = list.at(0).toShort();
    short j = list.at(1).toShort();
    task->excludedHole[i][j] = !task->excludedHole[i][j];
    if (task->excludedHole[i][j] == true)
        tabletWidget->h[i][j]->setStyleSheet("background-color: blue;");
    else
        tabletWidget->h[i][j]->setStyleSheet("background-color: green;");
}

void MethodSetup::onLeftClicked()
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

void MethodSetup::onRightClicked()
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

void MethodSetup::onContinueClicked()
{
    if (status != NOTRUNNING)
    {
        setCurrentIndex(0);
        return;
    }
    emit bottomLimit(task->cameraBottomLimit);                                          // Установить нижнюю границу оси Z для выбранного планшета
    emit calibrate(task);

    int exitCode = 0;
    if (task->temperatureSet !=0)
    {
        emit setTermostat(task->temperatureSet);                                        // !!!
        qmsg.setWindowTitle(tr("Термостабилизация"));
        qmsg.setText(tr("Задана стабилизация тем-\n"  \
                        "пературы эксперимента.  \n"  \
                        "Ожидайте достижения     \n"  \
                        "требуемого значения.    \n"  \
                        "Нажмите \"Ок\" для немед-\n" \
                                "ленного продолжения"));
        qmsg.setFocusPolicy(Qt::NoFocus);
        qmsg.setCursor(Qt::BlankCursor);
        exitCode = qmsg.exec();                                                         // !!!  Возвращает Rejected (0) или Close(1024)
        qDebug()<<exitCode;
    }
    ui->pbCam->setDisabled(task->doingShake);
    setCurrentIndex(0);
    if (exitCode == QMessageBox::Rejected)                                              // !!!
        onStartClicked();
}

void MethodSetup::onNextClicked()
{
    if (status == INPROCESS || status == PAUSED)
    {
        setCurrentIndex(2);
        return;
    }
    switch (task->tabletColumns) {
    case 4:
        tabletWidget->setTablet12Holes();
        break;
    case 6:
        tabletWidget->setTablet24Holes();
        break;
    case 8:
        tabletWidget->setTablet48Holes();
        break;
    case 12:
        tabletWidget->setTablet96Holes();
        break;
    default:
        break;
    }

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

    showHoleSelector();
}

void MethodSetup::onBackClicked()
{
    switch (status) {
    case NOTRUNNING:
        startPrepare();
        emit toMainReturn();
        break;
//    case INPROCESS:

//        break;
//    case PAUSED:

//        break;
    default:
        if (QMessageBox::Yes == QMessageBox::question(this,                                  // Ожидаем реакции пользователя на предупреждение о необходимости калибровки
                                            tr("Предупреждение"),
                                            tr("Желаете прервать\n эксперимент ?\n"),
                                            QMessageBox::Yes|QMessageBox::No))
        {
            task->continueFlag = false;
            ui->pbBack->setDisabled(true);
        }
        break;
    }
}

void MethodSetup::onStartClicked()
{
    switch (status) {
    case NOTRUNNING:
        expInProcess(true);
        emit letsStart(task);
        break;
    case INPROCESS:
        ui->pbStart->setText(tr("Продолжить"));
        status = PAUSED;
        task->pauseEnabled = true;
        ui->pbEject->setDisabled(false);
        break;
    case PAUSED:
        ui->pbStart->setText(tr("Приостановить"));
        status = INPROCESS;
        task->pauseEnabled = false;
        ui->pbEject->setDisabled(true);
        break;
    default:
        break;
    }
}

void MethodSetup::onEjectClicked()
{
    task->openTray = true;
    ui->pbEject->setDisabled(true);
}

void MethodSetup::onControlClicked()
{
    showMethodCtrl();
}

void MethodSetup::showMethodCtrl()
{
    setCurrentIndex(1);
}

void MethodSetup::showHoleSelector()
{
    setCurrentIndex(2);
}

void MethodSetup::updateCtrlState()
{
    ui->sbInterval->setValue(task->shotInterval / 1000);
    ui->sbTermostat->setValue(task->temperatureSet);
    ui->cbSaveToCloud->setChecked(task->sendToCloud);
}

void MethodSetup::changedCtrl()
{
    task->sendToCloud = ui->cbSaveToCloud->isChecked();
    task->shotInterval = ui->sbInterval->value() * 1000;
    task->temperatureSet = ui->sbTermostat->value();
}

void MethodSetup::onHolesClicked()
{
    ui->pbContinue->setText(tr("Закрыть"));
    showHoleSelector();
}

void MethodSetup::onCamClicked()
{
    emit videoControl(task->continueFlag);
}

void MethodSetup::expInProcess(bool flag)
{
    if (!flag)
    {
        startPrepare();
        emit videoControl(task->continueFlag);
    }
    else
    {
        status = INPROCESS;
        ui->pbContinue->setText(tr("Закрыть"));
        ui->pbStart->setText(tr("Приостановить"));
        ui->pbBack->setText(tr("Прервать"));
        ui->pbEject->setDisabled(true);
    }
}

void MethodSetup::startPrepare()
{
    status = NOTRUNNING;
    ui->pbStart->setText(tr("Начать"));
    ui->pbStart->setDisabled(false);
    ui->pbBack->setText(tr("К выбору метода"));
    ui->pbBack->setDisabled(false);
    ui->pbContinue->setText(tr("Продолжить"));
    ui->pbEject->setDisabled(true);
    ui->pbCam->setDisabled(false);
    task->pauseEnabled = false;
    task->continueFlag = false;
    task->openTray = false;
    emit setTermostat(0);
}

