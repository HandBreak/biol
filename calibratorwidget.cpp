#include "calibratorwidget.h"
#include "ui_calibratorwidget.h"

#include <QDebug>

CalibratorWidget::CalibratorWidget(VideoWidget &video, QWidget *parent) :               // Инициализируется указателем на виджет видеозахвата
    QWidget(parent),
    ui(new Ui::CalibratorWidget)
{
    ui->setupUi(this);
    ui->rbX01->setChecked(true);                                                        // По умолчанию установить шаг перемещения = 0.1
    ui->pbNext->hide();                                                                 // По умолчанию скрыть кнопку "Дальше"
    updatePos = false;                                                                  // По умолчанию отключено обновление позиций первой и последней лунок в задании

    QStackedLayout *stackedLayout = new QStackedLayout;                                 // Слой элементов управления под видеоокном
    QGridLayout *gridLayout = new QGridLayout;                                          // Слой размещения элементов внутри видеоокна
    widget = new QWidget;                                                               // Создать виджет для размещения элементов управления исполнительными механизмами

    QPushButton *pbLeft = new QPushButton(QChar(8678));                                 // Элементы управления позицией камеры     (QChar(706))
    QPushButton *pbRight = new QPushButton(QChar(8680));                                // --                                      (QChar(707))
    QPushButton *pbUp = new QPushButton(QChar(8679));                                   // Элементы управления позицией планшета   (QChar(708))
    QPushButton *pbDown = new QPushButton(QChar(8681));                                 // --                                      (QChar(709))
    QPushButton *pbZoomOut = new QPushButton("-");                                      // Элементы управления высотой камеры
    QPushButton *pbZoomIn = new QPushButton("+");                                       // --

    //  -- Настройка стиля элементов управления внутри видеоокна     -->!
    QFont font = pbLeft->font();
    font.setPointSize(32);
    pbLeft->setFont(font);
    pbRight->setFont(font);
    pbUp->setFont(font);
    pbDown->setFont(font);
    pbZoomIn->setFont(font);
    pbZoomOut->setFont(font);
    pbLeft->setFlat(true);
    pbRight->setFlat(true);
    pbUp->setFlat(true);
    pbDown->setFlat(true);
    pbZoomOut->setFlat(true);
    pbZoomIn->setFlat(true);
    pbLeft->setStyleSheet("color: rgb(255,255,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbRight->setStyleSheet("color: rgb(255,255,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbUp->setStyleSheet("color: rgb(255,255,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbDown->setStyleSheet("color: rgb(255,255,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbZoomIn->setStyleSheet("color: rgb(255,0,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbZoomOut->setStyleSheet("color: rgb(255,0,0);" \
                          "background-color: rgba(0,255,0,30);");
    pbLeft->setFixedSize(40,80);
    pbRight->setFixedSize(40,80);
    pbUp->setFixedSize(80,40);
    pbDown->setFixedSize(80,40);
    pbZoomOut->setFixedSize(40,40);
    pbZoomIn->setFixedSize(40,40);
    //  -- Настройка стиля элементов управления внутри видеоокна     --<!

    //  -- Настройка стиля элементов отображения внутри видеоокна    -->!
    ui->lbHoleInfo->setStyleSheet("color: rgb(255,170,0);"  \
                                  "background-color: rgba(0,0,0,128);");
    ui->lbHoleName->setStyleSheet("color: rgb(255,170,0);"  \
                                  "background-color: rgba(0,0,0,128);");
    ui->lbTempInfo->setStyleSheet("color: rgb(255,170,0);"  \
                                  "background-color: rgba(0,0,0,128);");
    //  -- Настройка стиля элементов отображения внутри видеоокна    --<!

    //  -- Настройка поведения элементов управления внутри видеоокна -->!
    pbLeft->setAutoRepeat(true);
    pbLeft->setAutoRepeatInterval(50);
    pbLeft->setFocusPolicy(Qt::NoFocus);
    pbRight->setAutoRepeat(true);
    pbRight->setAutoRepeatInterval(50);
    pbRight->setFocusPolicy(Qt::NoFocus);
    pbUp->setAutoRepeat(true);
    pbUp->setAutoRepeatInterval(50);
    pbUp->setFocusPolicy(Qt::NoFocus);
    pbDown->setAutoRepeat(true);
    pbDown->setAutoRepeatInterval(50);
    pbDown->setFocusPolicy(Qt::NoFocus);
    pbZoomOut->setAutoRepeat(true);
    pbZoomOut->setAutoRepeatInterval(50);
    pbZoomOut->setFocusPolicy(Qt::NoFocus);
    pbZoomIn->setAutoRepeat(true);
    pbZoomIn->setAutoRepeatInterval(50);
    pbZoomIn->setFocusPolicy(Qt::NoFocus);
    //  -- Настройка поведения элементов управления внутри видеоокна --<!

    //  -- Размещение элементов управления внутри видеоокна          -->!
    gridLayout->addWidget(pbLeft,1,0, Qt::AlignCenter);
    gridLayout->addWidget(pbRight,1,2, Qt::AlignCenter);
    gridLayout->addWidget(pbUp,0,1, Qt::AlignCenter);
    gridLayout->addWidget(pbDown,2,1, Qt::AlignCenter);
    gridLayout->addWidget(pbZoomOut,0,2, Qt::AlignCenter);
    gridLayout->addWidget(pbZoomIn,2,2, Qt::AlignCenter);
    //  -- Размещение элементов управления внутри видеоокна          --<!

    widget->setLayout(gridLayout);                                                      // Установить слой с элементами управления на виджет
    ui->videoLayout->addLayout(stackedLayout);                                          // Добавить стек слоёв в область видеовиджета
    stackedLayout->addWidget(&video);                                                   // Добавить в стек слой видеопотока
    stackedLayout->addWidget(widget);                                                   // Добавить в стек слой с элементами управления
    stackedLayout->setStackingMode(QStackedLayout::StackAll);                           // Выбрать режим объединения всех слоёв
    stackedLayout->setCurrentIndex(1);                                                  // Выбрать активный слой с элементами управления

    QObject::connect(ui->pbAction, SIGNAL(clicked(bool)), this, SLOT(onActionClicked()));
    QObject::connect(ui->pbNext, SIGNAL(clicked(bool)), this, SLOT(onReturnClicked()));
    QObject::connect(ui->pbExit, SIGNAL(clicked(bool)), this, SLOT(onReturnClicked()));
    QObject::connect(pbRight, SIGNAL(clicked(bool)), this, SLOT(onRightClicked()));
    QObject::connect(pbLeft, SIGNAL(clicked(bool)), this, SLOT(onLeftClicked()));
    QObject::connect(pbUp, SIGNAL(clicked(bool)), this, SLOT(onUpClicked()));
    QObject::connect(pbDown, SIGNAL(clicked(bool)), this, SLOT(onDownClicked()));
    QObject::connect(pbZoomOut, SIGNAL(clicked(bool)), this, SLOT(onZoomOutClicked()));
    QObject::connect(pbZoomIn, SIGNAL(clicked(bool)), this, SLOT(onZoomInClicked()));

    qDebug() << "Calibrator widget createrd";
}

CalibratorWidget::~CalibratorWidget()
{
    delete widget;                                                                      // Удалить виджет с элементами управления
    delete ui;
    qDebug() << "Calibrator widget destroyed";
}

void CalibratorWidget::onReturnClicked()
{
    this->close();                                                                      // Закрываем текущее окно (при нажатии на "Выход")
    if (widget->isHidden() == true)
        emit resVisualChk();                                                            // Послать сигнал для сброса галки вызуального контроля, если виджет был принудительно закрыт
    emit toMainReturn();                                                                // Послать сигнал открытия основного окна (объект MainWindow)
}

void CalibratorWidget::onRightClicked()
{
    if (ui->rbX01->isChecked())
        emit shiftX(100);
    else
        emit shiftX(1000);
}

void CalibratorWidget::onLeftClicked()
{
    if (ui->rbX01->isChecked())
        emit shiftX(-100);
    else
        emit shiftX(-1000);
}

void CalibratorWidget::onUpClicked()
{
    if (ui->rbX01->isChecked())
        emit shiftY(-100);
    else
        emit shiftY(-1000);
}

void CalibratorWidget::onDownClicked()
{
    if (ui->rbX01->isChecked())
        emit shiftY(100);
    else
        emit shiftY(1000);
}

void CalibratorWidget::onZoomInClicked()
{
    if (ui->rbX01->isChecked()) {
        emit shiftZ(-100);
    }
    else {
        emit shiftZ(-1000);
    }
}

void CalibratorWidget::onZoomOutClicked()
{
    if (ui->rbX01->isChecked()) {
        emit shiftZ(100);
    }
    else {
        emit shiftZ(1000);
    }
}

void CalibratorWidget::onActionClicked()
{
    emit pauseClicked();
}

void CalibratorWidget::showVideoViewer()
{
    this->ui->pbExit->show();
    this->ui->pbNext->hide();
    this->ui->pbAction->show();
    this->ui->pbAction->setText(tr("Пауза"));
    this->ui->groupBox->setHidden(true);
    this->ui->groupBox->setDisabled(true);
    this->ui->lbHoleInfo->setHidden(false);
    this->ui->lbHoleName->setHidden(false);
    this->ui->lbHoleInfo->setText("");
    this->ui->lbTempInfo->setHidden(false);
    this->widget->setHidden(true);
    this->showFullScreen();
}

void CalibratorWidget::showCalibratorCtl()
{
    this->ui->pbNext->setText(tr("Закрыть"));
    this->ui->pbNext->show();
    this->ui->pbExit->hide();
    this->ui->pbAction->hide();
    this->ui->groupBox->setHidden(false);
    this->ui->groupBox->setEnabled(true);
    this->ui->lbHoleInfo->setHidden(true);
    this->ui->lbHoleName->setHidden(true);
    this->ui->lbTempInfo->setHidden(true);
    this->widget->setHidden(false);
    this->showFullScreen();
}

void CalibratorWidget::setHoleName(QString holeInfo)
{
    this->ui->lbHoleInfo->setText(holeInfo);
}

void CalibratorWidget::setCurrentTemp(short temp)
{
    this->ui->lbTempInfo->setText(QString("%1%2°C").arg(temp >= 0 ? "+":"-").arg(temp));// Устанавливаеми значение полученной из сигнала температуры отображаемой лунки
}

void CalibratorWidget::calibrate(Task *task)
{
    currentTask = task;                                                                 // Инициализируем указатель на объект с параметрами задания
    QObject::disconnect(ui->pbNext, SIGNAL(clicked(bool)), this, SLOT(onReturnClicked()));
    if (QMessageBox::No == QMessageBox::question(this,                                  // Ожидаем реакции пользователя на предупреждение о необходимости калибровки
                                        tr("Предупреждение"),
                                        tr("Перед экспериментом\n"    \
                                           "необходимо убедиться\n"   \
                                           "в том, что планшет\n"     \
                                           "корректно откалиброван,\n"\
                                           "если уверены в этом,\n"   \
                                           "нажмите \"Да\"\n"),
                                           QMessageBox::Yes|QMessageBox::No))
    {
        int holeLastXPositoon = task->holeFirstXPosition + task->holeXSpacing * (task->tabletColumns - 1);
        int holeLastYPositoon = task->holeFirstYPosition + (task->holeYSpacing * -1) * (task->tabletRows - 1);
        emit goToPosition(holeLastXPositoon, \
                          holeLastYPositoon, \
                          task->cameraPosition);
        showCalibratorCtl();                                                            // Переключаем видеовиджет в режим калибровки (включаем элементы управления)
        ui->pbNext->setText(tr("Далее"));                                               // Переопределяем название кнопки "Выход"
        QMessageBox::information(0,tr("Калибровка. Шаг 1"), \
                                 tr("Установите камеру точно \n"  \
                                    "над нижней правой лункой\n"  \
                                    "настройте размер и фокус\n"  \
                                    "и нажмите кнопку \"Ок\"\n"));                      // Информируем пользователя о необходимых с его стороны действиях
        waiting();                                                                      // Ожидаем завершения позиционирования, инициированного перед уведомлением о первом шаге калибровки
        emit lightOn();                                                                 // Включаем подстветку после остановки механизмов
        QObject::connect(ui->pbNext, SIGNAL(clicked()), &loop, SLOT(quit()));           // Подключаем кнопку "Далее" для прерывания цикла ожидания
        loop.exec();                                                                    // Ждём действий пользователя
        updatePos = true;                                                               // Установить флаг обновления позиции лунки
        firstHole = false;                                                              // Сбросить флаг, указывающий на обновление позиции первой лунки (сначала устанавливалась позиция последней лунки)
        emit getCurrentPosition();                                                      // Запрашиваем актуальные координаты для откалиброванной последней лунки
        emit lightOff();                                                                // Гасим подстветку перед перемещением к первой лунке
        emit goToPosition(task->holeFirstXPosition, \
                          task->holeFirstYPosition, \
                          task->cameraPosition);                                        // Запрос на перемещение в первую позицию
        ui->pbNext->setText(tr("Готово"));                                              // Теперь кнопка "Далее" заменяется на "Готово" (Калибровка завершится и перейдем к ожиданию нагрева и опыту)
        QMessageBox::information(0,tr("Калибровка. Шаг 2"), \
                                 tr("Установите камеру точно \n"  \
                                    "над верхней левой лункой\n"  \
                                    "проверьте размер и фокус\n"  \
                                    "и нажмите кнопку \"Ок\"\n"));                      // Информируем пользователя о необходимых с его стороны действиях
        waiting();                                                                      // Ожидаем завершения позиционирования, инициированного перед уведомлением о втором шаге калибровки
        emit lightOn();                                                                 // Включаем подстветку после остановки механизмов
        loop.exec();                                                                    // Ждём действий пользователя
        updatePos = true;                                                               // Установить флаг обновления позиции лунки
        firstHole = true;                                                               // Установить флаг, указывающий на обновление позиции первой лунки (сначала устанавливалась позиция последней лунки)
        emit getCurrentPosition();                                                      // Запрашиваем актуальные координаты для откалиброванной первой лунки
        this->hide();                                                                   // Скрываем виджет калибровки перед выходом
        QObject::disconnect(ui->pbNext, SIGNAL(clicked()), &loop, SLOT(quit()));        // Отключаем кнопку "Далее/Готово" от цикла ожидания
        QObject::connect(this, SIGNAL(go()), &loop, SLOT(quit()));                      // Обеспечить прерывание цикла ожидания по сигналу об окончании корректировки координат в задании
        loop.exec();                                                                    // Цикл ожидания сигнала GO от axisPosition (установки скорректированных значений шага и стартовой позиции
        QObject::disconnect(this, SIGNAL(go()), &loop, SLOT(quit()));                   // Отключить связь с сигналом перед выходом и восстановить ранее отключенную связь кнопки "Выход" с закрытием виджета
        QObject::connect(ui->pbNext, SIGNAL(clicked(bool)), this, SLOT(onReturnClicked()));
    }
}

void CalibratorWidget::axisPosition(int x, int y, int z)
{
    if (updatePos == false)                                                             // Ничего не делать, если не установлен флаг обновления шага и положений первой и последней лунок в задании
        return;
    updatePos = false;                                                                  // Если флаг был установле, сбрасывем его и переходим к коррекции координат
    if (x == 0 || y == 0 || z == 0)                                                     // Позиции не могут иметь нулевое значение, это признак ошибки чтения данных от Arduino
    {
        qDebug() << "ERROR NULL";                                                       // Во избежание повреждения механизмов из-за некорректных координат прерываем работу виджета калибровки
        qDebug() << "X:" << x << " Y:" << y << " Z:" << z;                              // и сообщаем в консоль о возникшей ошибке
        qDebug() << tr("Пришло время самоубиться");
        delete this;
    }
    if (firstHole == true)                                                              // Если пришли координаты первой лунки, осуществляем рассчет и установку значений
    {
        currentTask->holeFirstXPosition = x;                                            // первой лунки по всем трем осям без изменений
        currentTask->holeFirstYPosition = y;                                            //
        currentTask->cameraPosition = z;                                                //

        // Далее производим расчет шага по осям X и Y и высоты подъема камеры (как среднее арифметическое установленной для первой и последней лунок
        currentTask->holeXSpacing = (holeLastXPosition - x) / (currentTask->tabletColumns - 1);
        currentTask->holeYSpacing = -1 * (holeLastYPosition - y) / (currentTask->tabletRows - 1);
        currentTask->cameraPosition = (holeLastZPosition + z) / 2;

        qDebug() << "Xs:" << currentTask->holeXSpacing << " Ys:" << currentTask->holeYSpacing << " Zc:" << currentTask->cameraPosition;
        qDebug() << "Xf:" << currentTask->holeFirstXPosition << " Yf:" << currentTask->holeFirstYPosition << " Zc:" << currentTask->cameraPosition;
        qDebug() << "Xl:" << holeLastXPosition << " Yl:" << holeLastYPosition << " Zl:" << holeLastZPosition;
        qDebug();
        emit go();                                                                      // Посылаем сигнал о завершении корректировки задания и возможности перейти к выполнению эксперимента
    }
    else                                                                                // Если пришли координаты последней лунки, сохраним их в промежуточных переменных для соответствующих осей
    {
        holeLastXPosition = x;
        holeLastYPosition = y;
        holeLastZPosition = z;
    }
}

void CalibratorWidget::waitMoving(bool s)                                               // Слот для приёма сигнала о состоянии исполнительных устройств (в работе / выполнено)
{
    if (s == false)
        emit go();                                                                      // Вызываем прерывание циклов ожидания, как только завершена работа исполнительных устройств
}

void CalibratorWidget::waiting()                                                        // Функция ожидания позиционировани исполнительных механизмов
{
    QTimer *timer = new QTimer();                                                       // Таймер опроса исполнительных механизмов
    timer->setSingleShot(false);                                                        // В цикли ожидания по таймеру каждые 0,5 сек отправляем запрос состояния исполнительных механизмов
    QObject::connect(timer, SIGNAL(timeout()), this, SIGNAL(getMoveStatus()));
    QObject::connect(this, SIGNAL(go()), &loop, SLOT(quit()));                          // Сигнал о готовности ИУ завершает цикл ожидания
    timer->start(500);
    loop.exec();
    QObject::disconnect(this, SIGNAL(go()), &loop, SLOT(quit()));                       // Отключает приём сигналов в цикл ожидания
    timer->stop();                                                                      // Отключает таймер опроса исполнительных механизмов
    QObject::disconnect(timer, SIGNAL(timeout()), this, SIGNAL(getMoveStatus()));       // Отключает передачу запросов от таймера к исполнительным механизмам
    delete timer;                                                                       // Удаляет таймер
}


