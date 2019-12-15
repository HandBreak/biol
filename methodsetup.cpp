#include "methodsetup.h"
#include "ui_methodsetup.h"

MethodSetup::MethodSetup(QWidget *parent, Task *t) :
    QStackedWidget(parent),
    ui(new Ui::MethodSetup)
{
    this->task = t;
    ui->setupUi(this);
    ui->rbHoleBacklight->setChecked(true);
    ui->sbInterval->setRange(0,60);
    ui->sbTermostat->setRange(0,50);
    tabletWidget = new TabletWidget;
    ui->pgHoleSelector->setLayout(tabletWidget->stLayout);

    setCurrentIndex(0);

    QObject::connect(ui->pbLeft, SIGNAL(clicked()),this, SLOT(onLeftClicked()));
    QObject::connect(ui->pbRight, SIGNAL(clicked()),this, SLOT(onRightClicked()));
    QObject::connect(ui->pbNext, SIGNAL(clicked()),this ,SLOT(onNextClicked()));
    QObject::connect(ui->pbContinue, SIGNAL(clicked()),this, SLOT(onContinueClicked()));
    QObject::connect(ui->pbBack, SIGNAL(clicked()), this, SLOT(onBackClicked()));
    QObject::connect(ui->pbPause, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
    QObject::connect(ui->cbSaveToCloud, SIGNAL(clicked()),this, SLOT(changedCtrl()));
    QObject::connect(ui->sbInterval, SIGNAL(valueChanged(int)),this, SLOT(changedCtrl()));
    QObject::connect(ui->sbTermostat, SIGNAL(valueChanged(int)),this, SLOT(changedCtrl()));
    QObject::connect(ui->rbDisableCtrl, SIGNAL(clicked()),this, SLOT(changedVisualCtrl()));
    QObject::connect(ui->rbVisualCtrl, SIGNAL(clicked()),this, SLOT(changedVisualCtrl()));
    QObject::connect(ui->rbHoleBacklight, SIGNAL(clicked()),this, SLOT(changedVisualCtrl()));
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
    if (ui->pbContinue->text() == "Закрыть")
    {
        setCurrentIndex(0);
        ui->rbDisableCtrl->setChecked(true);
        ui->pbContinue->setText(tr("Продолжить"));
        return;
    }
    ui->pbContinue->setText(tr("Закрыть"));
    emit bottomLimit(task->cameraBottomLimit);                          // Установить нижнюю границу оси Z для выбранного планшета
    emit calibrate(task);
    if (task->temperatureSet !=0)
    {
//        emit setTermostat(task->temperatureSet);
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

void MethodSetup::onNextClicked()
{
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
    task->continueFlag = false;
    emit toMainReturn();
}

void MethodSetup::onPauseClicked()
{
    task->pauseEnabled = !task->pauseEnabled;
    if (task->pauseEnabled)
        ui->pbPause->setText(tr("Продолжить"));
    else
        ui->pbPause->setText(tr("Приостановить"));
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
    ui->cbSaveToCloud->setChecked(task->sendToCloud);
    ui->sbInterval->setValue(task->shotInterval / 1000);
    ui->sbTermostat->setValue(task->temperatureSet);
}

void MethodSetup::changedCtrl()
{
    task->sendToCloud = ui->cbSaveToCloud->isChecked();
    task->shotInterval = ui->sbInterval->value() * 1000;
    task->temperatureSet = ui->sbTermostat->value();
}

void MethodSetup::changedVisualCtrl()
{
    if (ui->rbHoleBacklight->isChecked() && task->continueFlag)
    {
        ui->pbContinue->setText(tr("Закрыть"));
        showHoleSelector();
    }
    else if (ui->rbVisualCtrl->isChecked() && task->continueFlag)
        emit videoControl(true);
}

void MethodSetup::onCloseVisualCtl()
{
    ui->rbDisableCtrl->setChecked(true);
}
