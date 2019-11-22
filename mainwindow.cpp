#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QProcess *p = new QProcess();
    p->start("cat /etc/MAC");                                                           // Получаем MAC-адрес Ethernet-контроллера. Последние 2 байта используем в качестве префикса имен файлов
    p->waitForFinished();
    mac = p->readAll();

    // Arduino thread
    arduino = new Arduino(actInterface);
    arduinoThread = new QThread();
    arduino->moveToThread(arduinoThread);

    // Termostat thread
    termostat = new Termostat(arduino);
    termoThread = new QThread();
    termostat->moveToThread(termoThread);

    // TaskExecuter thread
    taskExecutor = new TaskExecutor(arduino);                                           // Инициализируем объект управления исполнительными механизмами
    taskExecutorThread = new QThread();                                                 // Инициализируем поток в который будем переностить работу с COM-портом
    taskExecutor->moveToThread(taskExecutorThread);                                     // Переносим объект взаимодействия с исполнительными устройствами в новый поток

    // WebDAV sender
    webdav = new Cloud(CLOUDUSR, CLOUDPWD, CLOUDURL);                                   // Инициализируем WeDAV-поток, используя дефолтные значения аутентификации и URL (из Actuatorconstants) Изменть!!!
    webdavThread = new QThread();
    webdav->moveToThread(webdavThread);
    //
    calibratorWidget.setVideoWidget(videoWidget);
    videoWidget.setMinimumSize(240, 240);                                               // Установить минимальный размер виджета
    videoCapture = new CaptureThread(&videoWidget);
    videoCapture->setDevice("/dev/video0");                                             // Имя устройства захвата. Перенести в конфигуратор и реализовать возможность выбора из списка !!!
    videoCapture->capture = true;                                                       // Завершив инициализацию устройства, разрешаем цикл захвата.

    oExperiments = new Experiments(0, &task);                                           // Инициализируем объект настройки опыта

    this->addWidget(&netSettings);
    this->addWidget(oExperiments);
//    this->addWidget(&calibratorWidget);

    tsTimer.setSingleShot(false);
    tsTimer.setInterval(500);                                                           // Интервал обновления состояния термостата. (0,5сек. Оптимизировать!!!)

    setCurrentIndex(MAINMENU);

    // Приём сообщений от объекта управления исполнительными механизмами
    QObject::connect(arduino, SIGNAL(homed(bool)), this, SLOT(homed(bool)));
    QObject::connect(arduino, SIGNAL(executeError(int)), this, SLOT(errard(int)));

    // Трансляция состояний от объекта управления исполнительными механизмами к виджету калибровки
    QObject::connect(arduino, SIGNAL(movingInProcess(bool)), &calibratorWidget, SLOT(waitMoving(bool)));
    QObject::connect(arduino, SIGNAL(axisPosition(int,int,int)), &calibratorWidget, SLOT(axisPosition(int,int,int)));
    QObject::connect(arduino, SIGNAL(extruderTemperature(short)), &calibratorWidget, SLOT(setCurrentTemp(short)));

    // Передача команд объекту управления исполнительними механизмами (для инициализации и завершения работы)
    QObject::connect(this, SIGNAL(doHoming()), arduino, SLOT(homing()));
    QObject::connect(this, SIGNAL(heaterOff()), arduino, SLOT(heatingOff()));
    QObject::connect(this, SIGNAL(lightOff()), arduino, SLOT(lightOff()));
    QObject::connect(this, SIGNAL(coolerOff()), arduino, SLOT(coolerOff()));

    // Вызов функций модулей из меню основного интерфейса. В том числе вызов самих модулей.  Лишнее убрать!!!
    QObject::connect(ui->pbEmrgStop, SIGNAL(clicked()), arduino, SLOT(emergencyStop()));
    QObject::connect(ui->pbTable, SIGNAL(clicked()), taskExecutor, SLOT(changeTablet()));
    QObject::connect(ui->pbExpMenu, SIGNAL(clicked()),this, SLOT(onExperimentsClicked()));
    QObject::connect(ui->pbNetSettings, SIGNAL(clicked(bool)), this, SLOT(onNetSettingsClicked()));
    QObject::connect(ui->pbCamera, SIGNAL(clicked(bool)), &calibratorWidget, SLOT(showCalibratorCtl()));

    // Вызов страницы настроек из меню основного интерфейса.
    QObject::connect(ui->pbMainWindow, SIGNAL(clicked()), SLOT(onMainClicked()));
    QObject::connect(ui->pbSettings, SIGNAL(clicked()), SLOT(onSettingsClicked()));

    // Отработка сигналов возврата в основное меню из модулей. Убрать после объединения !!!
    QObject::connect(oExperiments, SIGNAL(toMainReturn()), this, SLOT(onMainClicked()));
    QObject::connect(&netSettings, SIGNAL(toMainReturn()), this, SLOT(onMainClicked()));

    // Отработка сигналов от модуля управления экспериментом
    QObject::connect(oExperiments, SIGNAL(letsStart(Task*)), taskExecutor, SLOT(startExperiment(Task*)));  // Передаёт исполнителю Задание
    QObject::connect(oExperiments, SIGNAL(bottomLimit(int)), arduino, SLOT(setBottomLimit(int)));          // Передаёт объекту управления ИМ допустимую нижнюю границу по Z для типа планшета
    QObject::connect(oExperiments, SIGNAL(videoControl(bool)), this, SLOT(videoControlMode(bool)));        // Позволяет включать/отключать видеоконтроль во время опыта
    QObject::connect(oExperiments, SIGNAL(calibrate(Task*)), &calibratorWidget, SLOT(calibrate(Task*)));    // Передаёт задание калибратору перед началом опыта

    // Обеспечение обмена сообщениями с демоном термостата и его циклический вызов по таймеру
    QObject::connect(&tsTimer, SIGNAL(timeout()), termostat, SLOT(update()));
    QObject::connect(oExperiments, SIGNAL(setTermostat(short)), termostat, SLOT(setTemperature(short)));
    QObject::connect(taskExecutor, SIGNAL(setTermostat(short)), termostat, SLOT(setTemperature(short)));
    QObject::connect(termostat, SIGNAL(heated()), &oExperiments->qmsg, SLOT(close()));
    // QObject::connect(termostat, SIGNAL(heatUp(short)), oExperiments, SLOT(dispTemp(short)));

    // Приём сообщений от исполнителя заданий. (Состояние эксперимента и описание кадров)
    QObject::connect(taskExecutor, SIGNAL(experimentInProgress(bool)), oExperiments, SLOT(waitEndOfExperiment(bool)));
    QObject::connect(taskExecutor, SIGNAL(experimentInProgress(bool)), this, SLOT(onExperimentInProgress(bool)));
    QObject::connect(taskExecutor, SIGNAL(takeShot(QStringList)), this, SLOT(onShotSignal(QStringList)));

    // Трансляция команд от виджета калибровки объекту управления исполнительными механизмами.  Убрать после объединения!!!
    QObject::connect(&calibratorWidget, SIGNAL(shiftX(int)), arduino, SLOT(shiftX(int)));
    QObject::connect(&calibratorWidget, SIGNAL(shiftY(int)), arduino, SLOT(shiftY(int)));
    QObject::connect(&calibratorWidget, SIGNAL(shiftZ(int)), arduino, SLOT(shiftZ(int)));
    QObject::connect(&calibratorWidget, SIGNAL(lightOn()), arduino, SLOT(lightOn()));
    QObject::connect(&calibratorWidget, SIGNAL(lightOff()), arduino, SLOT(lightOff()));
    QObject::connect(&calibratorWidget, SIGNAL(getMoveStatus()), arduino, SLOT(sendMoveStatus()));
    QObject::connect(&calibratorWidget, SIGNAL(getCurrentPosition()), arduino, SLOT(sendCurrentPosition()));
    QObject::connect(&calibratorWidget, SIGNAL(goToPosition(int,int,int)), arduino, SLOT(setAPosition(int,int,int)), Qt::BlockingQueuedConnection); // установить в положение калибровки
    QObject::connect(&calibratorWidget, SIGNAL(pauseClicked()), this, SLOT(pauseClicked()));

    // Трансляция сигнала о закрытии видеоконтроля в интерфейс управляения экспериментом
    QObject::connect(&calibratorWidget, SIGNAL(resVisualChk()), oExperiments, SLOT(onCloseVisualCtl()));

    // Трансляция имени ячейки в виджет видеоконтроля и виджет выбора лунок для подсветки снимаемой
    QObject::connect(taskExecutor, SIGNAL(holeName(QString)), &calibratorWidget, SLOT(setHoleName(QString)));
    QObject::connect(taskExecutor, SIGNAL(holeName(QString)), oExperiments, SLOT(curHoleNumber(QString)));

    // WebDAV Инициализация, закрытие и передача списка загружаемых файлов.
    QObject::connect(this, SIGNAL(sendToCloud(QString)), webdav, SLOT(curlSendFile(QString)));
    QObject::connect(this, SIGNAL(cloudConnInit(bool)), webdav, SLOT(curlOpenConn(bool)));
    QObject::connect(this, SIGNAL(cloudConnClose()), webdav, SLOT(curlCloseConn()));

    webdavThread->start();                                                              // Запустить поток отправки данных в облако
    taskExecutorThread->start();
    termoThread->start();                                                               // Запустить поток термостата
    arduinoThread->start();                                                             // Запустить поток обработки команд для контроллера ИМ.

    tsTimer.start();                                                                    // Запустить таймер циклического вызова термостата

    emit heaterOff();
    emit lightOff();
    emit coolerOff();
    emit doHoming();                                                                    // Отключить все устройства и произвести калибровку механизмов перед началом работы
}

MainWindow::~MainWindow()
{
    videoCapture->capture = false;                                                      // Перед завершением работы остановить цикл захвата видео
    videoCapture->quit();
    videoCapture->wait();

    delete videoCapture;
    delete oExperiments;
    delete taskExecutor;
    taskExecutorThread->quit();
    taskExecutorThread->wait();
    delete taskExecutorThread;

    delete termostat;
    termoThread->quit();
    termoThread->wait();
    delete termoThread;

    delete arduino;
    arduinoThread->quit();
    arduinoThread->wait();
    delete arduinoThread;

    delete ui;

    // Удаляем WebDAV sender
    delete webdav;
    delete webdavThread;

    qDebug() << "MainWindow destroyed!";
}

void MainWindow::onShotSignal(QStringList tagInfo)                                      // Слот вызова захвата кадра и его сохранения во временный каталог с полученными тэгами условий съемки
{
    videoCapture->takeshot = true;                                                      // Запрашиваем cохранение в объект videoCapture->cs текущего кадра
    do
    {
        usleep(10);
    } while (videoCapture->takeshot);                                                   // Ждем готовности снимка. Требуется контроль таймаута. АКТИВНОЕ ОЖИДАНИЕ !!!
    QTextStream record(&filelist);                                                      // Текстовый поток для формирования списка имён файлов
    QString filename = mac.mid(12,5).replace(":","") + QString("-") + QTime::currentTime().toString("HHmmss.zzz");
    if (asynchronous)
    {
        /* Асинхронный режим (true) не прерывает съемку на время сохранения снимков на диск */
        QString fullname = path + filename + QString(".png");
        while (!tagInfo.isEmpty())
        {
            QString string = tagInfo.first();
            tagInfo.pop_front();
            QString key = string.left(string.indexOf(","));
            QString value = string.right(string.size() - string.indexOf(",") - 1);
            videoCapture->cs.setText(key, value);
        }
        taskExecutor->wait= false;                                                      // Завершать ожидание сохранения файла не дожидаясь завершения процесса (не ждать)
        if (!videoCapture->cs.save(fullname, "png", 100))                               // БЕЗ СЖАТИЯ, ДОСТАТОЧНО МЕДЛЕННО !!! (85 - со сжатием, но очень медленно)
        {
            qDebug() << "Shot not saved, write error!";
            task.continueFlag = false;                                                 // В случае ошибки записи сбросить флаг, признак продолжения опыта
        }
    }
    else
    {
        /* Синхронный режим (false). На время сохранения снимка на диск зажание приостанавливается */
        QString fullname = path + filename + QString(".bmp");
        if (!videoCapture->cs.save(fullname, "bmp"))                                    // БЕЗ СЖАТИЯ И ТЭГОВ, БЫСТРЫЙ МЕТОД
        {
            qDebug() << "Shot not saved, write error!";
            task.continueFlag = false;                                                 // В случае ошибки записи сбросить флаг, признак продолжения опыта
        }
        QFile file(fullname);
        if (!file.open(QIODevice::Append | QIODevice::Text))
        {
            qDebug() << "Shot file open error!";
            task.continueFlag = false;                                                 // В случае ошибки записи сбросить флаг, признак продолжения опыта
        }
        QTextStream stream(&file);                                                      // Текстовый поток для дозаписи в конец изображения тэгов с параметрами съемки
        QString line("tEXt");
        stream << line;
        while (!tagInfo.isEmpty())
        {
            QString string = tagInfo.first();
            tagInfo.pop_front();
            QString key = string.left(string.indexOf(","));
            QString value = string.right(string.size() - string.indexOf(",") - 1);
            stream << (key + QString(";") + value + QString("\n")).toAscii();
        }
        dsize = dsize + file.size();                                                    // Подсчет дискового пространства, необходимого для последующего копирования на внешний носитель
        file.close();
        if (stream.status() != QTextStream::Ok)
        {
            qDebug() << "Shot metadata write error!";
            task.continueFlag = false;                                                 // В случае ошибки записи метаданных сбросить флаг, признак продолжения опыта
        }
        else
            /* Сброс файла в "облако" WebDAV, если в задание предполагает передачу в NextCloud */
            if (task.sendToCloud)
            {
                sendcounter++;                                                          // Счетчик отправленных файлов
                emit sendToCloud(fullname);
            }
    }
    taskExecutor->wait = false;                                                         // Завершить ожидание сохранения файла (синхронный режим)
    shotcounter++;                                                                      // Подсчет количества сохраненных снимков
    record << (filename + QString("\r\n")).toAscii();                                   // Дописать строку с именем файла в текстовый поток 'description.id'
    qDebug()<<"SHOT!"<<endl;
}

void MainWindow::onExperimentInProgress(bool process)                                   // Слот вызова процедур управления файлами резултатов / TODO: Подсчет необходимого места на диске и сверка!
{
    QString basepath = "/tmp/pictures/";                                                // Временный каталог - создается запускающим скриптом и туда монтируется SSD-накопитель
    QString tempfolder = "experiment.tmp";                                              // Временный подкаталог - в конце эксперимента переименовывается текущей датой

    if (process)
    {
        /* Выполняется в начале эксперимента. Инициализация соединений, подготовка хранилища */
        sendcounter = 0;                                                                // Обнулим счетчик переданных
        if (task.sendToCloud)
            emit cloudConnInit(true);                                                   // Откроем соединение, если включена передача в NextClowd
        dsize = 0;                                                                      // Обнулим значение счетчика необходимого места на съемном носителе
        shotcounter = 0;
        if (usetempfolder)
        {
            /* Установлен флаг использования временных подкаталогов для обработки файлов внешними скриптами */
            path = basepath + tempfolder + "/";
            QDir newdir(basepath);
            if (!newdir.mkdir(tempfolder))
            {
                qDebug() << "Create temporary folder error!";
                task.continueFlag = false;                                             // В случае ошибки создания подкаталога сбросить флаг, признак продолжения опыта
                return;
            }
        }
        else
            /* Использование временных подкаталогов отключено - обработка файлов собственными средствами */
            path = basepath;
        if (autoclean)
        {
            /* Установлен флаг автоудаления старых снимков при запуске нового опыта. */
            QDir dir(path);
            QString ext;
            if (asynchronous)
                ext = "png";                                                            // В асинхронном режиме файлы в png (медленно !!!)
            else
                ext = "bmp";                                                            // В синхронном режиме в bmp (быстро)
            QStringList list("*." + ext);
            list.append("description.*");
            dir.setNameFilters(list);
            dir.setFilter(QDir::Files);
            foreach (QString dirFile, dir.entryList()) {
                dir.remove(dirFile);                                                    // Очищаем каталог от старых файлов в соответствии со сформированным списком
            }
        }
        filelist.setFileName(path + QString("description.tmp"));
        if(!filelist.open(QIODevice::Append | QIODevice::Text))                         // Создаем новый файл описания снимков с открытием в режиме дозаписи
        {
            qDebug() << "Error create description file!";
            task.continueFlag = false;                                                 // В случае ошибки создания файла-дескриптора сбросить флаг, признак продолжения опыта
        }
        long kbytes = taskExecutor->holes *                                             // Умножаем расчетное число снимков на их объем
               (videoCapture->yres *                                                    // Вычисляемый как квадрат меньшей стороны
                videoCapture->yres * 3 + 200 ) / 1024;                                  // и добавляем размер служебных данных с запасом

        if (kbytes > checkFreeSpace(basepath))
        {
            qDebug() << "There may not be enough disk space!";
            QMessageBox::warning(0,
                                 tr("Внимание!"),
                                 tr("Может быть недостаточно места на диске!"));
            task.continueFlag = false;                                                 // В случае недостатка места на внутреннем хранилище сбосить флаг, признак продолжения опыта
            return;
        }
        QTextStream record(&filelist);
        record << task.methodCode << "\r\n";                                           // Записать название метода в заголовок файла-дескриптора из Задания.
    }
    else
    {
        /* Выполняется по завершению эксперимента - отработка задания закончена */
        QTextStream record(&filelist);
        record << shotcounter;                                                          // Дописать в конец файла-дескриптора числа сделанных снимков
        dsize = dsize +filelist.size();                                                 // Вычислить необходимое дисковое пространства для всех файлов эксперимента
        filelist.close();                                                               // Закрыть файл-дескриптор
        if (record.status() != QTextStream::Ok)
        {
            qDebug() << "Description list write error!";
            task.continueFlag = false;                                                 // В случае ошибки создания закрытия файла-дескриптора сбросить флаг, признак продолжения опыта
        }
        if (!QFile::rename(path + QString("description.tmp"), path + QString("description.id")))
            qDebug() << "Error rename description file!";

        if (task.sendToCloud)
        {
            /* Отправка файла-описания в WebDAV, если было включено в задании на эксперимент */
            sendcounter++;                                                              // Счетчик отправляемых в NextCloud файлов
            emit sendToCloud(path + QString("description.id"));
        }
        if (usetempfolder)
        {
            /* При включении "временных папок" переименовываем их датой и временем окончания опыта */
            QString newfolder = QDateTime::currentDateTime().toString("ddMMyy-HHmmss");
            path = basepath + newfolder + "/";                                          // !!! Удалить если не требуется в дальнейшем
            QDir newdir(basepath);
            if (!newdir.rename(tempfolder, newfolder))
            {
                qDebug() << "Rename folder error!";
            }
        }

        bool mustsave = true;                                                           // Флаг - требует обязательного сохранения на сменном носители (если не включено NextClown или возникла ошибка передачи)
        if (task.sendToCloud)
            /* Ожидаем завершения передачи в WebDAV и зарываем соединение, если было включено сохранение в облако */
            mustsave = waitForSending();                                                // Если при сохранение в NextCloud возникли ошибки, сохраняем флаг "Необходимо сохранить" в true
        saveToRemovable(mustsave);                                                      // Предложить сохранение на USB-носитель. Требовать, если 'mustsave = true'
    }
}

bool MainWindow::saveToRemovable(bool mustsave)                                         // Функция сохранения данных на съемный носитель
{
    bool success;                                                                       // Признак успеха операции
    bool umounted;                                                                      // Признак состояния внешнего носителя
    do
    {
        umounted = true;
        success = false;
        QMessageBox *qmsg = new QMessageBox(QMessageBox::Question,
                                            tr("Опыт завершен."),
                                            tr("Сохранить результаты на USB-накопитель ?"),
                                            QMessageBox::Yes | QMessageBox::No,
                                            0);

        //        qmsg->setParent(0);
        short answer = qmsg->exec();                                                    // Ожидаем реакцию пользователя на запрос сохранения результатов на съемный носитель
        delete qmsg;
        if (answer == QMessageBox::Yes)
        {
            /* Пользователь согласился сохранить данных на внешний носитель */
            if (driveMount() == true)                                                   // Пробуем смонтировать
            {
                /* Носитель уже смонтирован */
                if ((dsize / 1024) >= checkFreeSpace("/mnt"))
                {
                    /* Необходимый объем дискового пространства меньше имеющегося на смонтированном носителе */
                    QMessageBox::warning(0,tr("Ошибка копирования."),tr("Недостаточно места на съемном носителе!"));
                    umounted = driveUmount();                                           // Отмонтировать внешний носитель на котором недостаточно места
                    continue;                                                           // Вернуться к запросу сохранения результатов на внешний носитель
                }
                success = cpToRemovableDrive();                                         // success=true, если копирование завершилось успешно
                umounted = driveUmount();                                               // Отключить носитель, umounted=true в случае успеха
                if (!success)
                {
                    /* Возникла ошибка записи на внешний носитель, надо сообщить пользователю */
                    QMessageBox::warning(0,tr("Ошибка копирования."),tr("Не удалось скопировать один или несколько файлов!"));
                    continue;                                                           // Вернуться к запросу сохранения результатов на внешний носитель
                }
            }
            else
                /* Монтирование не удалось, возможно носитель не обнаружен */
                QMessageBox::warning(0,tr("Ошибка монтирования."),tr("Не удалось подключить съемный диск!"));
        }
        else
            /* Пользователь отказался от сохранения данных на внешний носитель */
            if (mustsave == true)
            {
                /* результаты должны быть сохранены на съемном носителе, так как в сеть не сохранялись/сохранить не удалось */
                QMessageBox *qmsg = new QMessageBox(QMessageBox::Question,
                                                    tr("Данные не сохранены!."),     \
                                                    tr("Если Вы откажитесь от\n"     \
                                                       "сохранения, результаты\n"    \
                                                       "опыта или их часть\n" \
                                                       "будут утрачены!\n"           \
                                                       "Вы уверены в выборе?\n" ),
                                                    QMessageBox::Yes | QMessageBox::No,
                                                    0);


                short answer = qmsg->exec();
                delete qmsg;
                if (answer == QMessageBox::Yes)
                    break;                                                              // Если пользователь настаивает на потере резултатов, прервать цикл сохранения
                else
                    continue;                                                           // но если одумался, дать еще один шанс - вернёмся к началу
            };
            break;                                                                      // Если данные были загружены в NextCloud, позволим выйти из цикла без успешного сохранения
    } while(!success);                                                                  // В случае неудачной попытки копирования запросим его вновь, иначе все ОК, выходим
    if (!umounted)
        /* Независимо ни от чего в конце процедуры носитель должен оказаться отключен */
        QMessageBox::warning(0,tr("Ошибка монтирования."),tr("Не удалось корректно отключить съемный диск!"));
    return success & umounted;                                                          // true говорит о полном успехе сохранения
}

bool MainWindow::driveMount()                                                           // Функция монтирования съемного носителя, возвращает true в случае успеха. Доработать логику кода !!!
/* РЕШИТЬ ПРОБЛЕМУ С ИМЕНОВАНИЕМ СЪЕМНОГО НОСИТЕЛЯ ПОДКЛЮЧЕННОГО ДО ЗАГРУЗКИ !!! */
{
    QProcess *p = new QProcess();
    bool success = false;
    p->start("cat /etc/mtab");
    p->waitForFinished();
    if (!QString(p->readAll()).contains("/mnt", Qt::CaseInsensitive))
    {
        /* Отсутствует точка монтирования в /mnt, попробуем что-нибудь подключить */
        short i;
        for (i = 1; i < 5 ; i++)
        {
            /* Переберем до 5 разделов на носителе. Зачем еще не придумал :) */
            if (QFile::exists("/dev/sdb" + QString::number(i)))
            {
                qDebug()<<"mount: "<<"/dev/sdb" + QString::number(i);
                p->start("mount /dev/sdb" + QString::number(i) + " /mnt");              // но монтируем первый по списку
                p->waitForFinished();
                QString output = p->readAllStandardError();
                qDebug() << output;
                if (output == "")
                    success = true;                                                     // Может следует пробовать монтировать другие разделы, если возникла ошибка ???
                break;                                                                  // Прерываем дальнейшие попытки монтирования с первого же подключенного раздела !!!
            }
        }
        if (i != 1 && QFile::exists("/dev/sdb"))
        {
            /* Для случаев, если цикл перебора разделов ушел дальше первого, подключаем носитель целиком !!! - А КАК ЛУЧШЕ ? */
            qDebug()<<"mount: "<<"/dev/sdb";
            p->start("mount /dev/sdb /mnt");
            p->waitForFinished();
            QString output = p->readAllStandardError();
            qDebug() << output;
            if (output == "")
                success = true;
        }
    }
    else
    {
        /* Точка монтирования уже существует, значит все ОК, но надо бы предупредить об этом факте */
        qDebug() << "already mounted";
        success = true;
    }
    return success;                                                                     // Возвращаем результат. Если было прервано, то false
}

bool MainWindow::driveUmount()                                                          // Функция отключения ранее смонтированного носителя, возвращает true, в случае успеха.
{
    QProcess *p = new QProcess();
    bool success = false;
    p->start("cat /etc/mtab");
    p->waitForFinished();
    if (QString(p->readAll()).contains("/mnt", Qt::CaseInsensitive))
    {
        p->start("umount /mnt");
        p->waitForFinished();
        QString output = p->readAllStandardError();
        qDebug() << output;
        if (output == "")
            success = true;
    }
    else
    {
        /* При отмонтировании в вывод посыпалась ругань, надо бы правильно обрабоать. Может занят носитель, а может уже отключен !!! - доработать */
        qDebug() << "already unmounted";
        success = true;
    }
    return success;
}

bool MainWindow::cpToRemovableDrive()                                                   // Функция копирования файлов из рабочей директории на схемный носитель. В случае успеха возвращает true
{
    bool success = false;
    QDir dir(path);
    QString ext;
    if (asynchronous)
        ext = "png";
    else
        ext = "bmp";
    QProgressBar *pbr = new QProgressBar(oExperiments);                                 // ProgressBar размещаем на диалоговом окне эксперимента - ДОРАБОТАТЬ !!! Были проблемы с собственным окном

    QStringList list("*." + ext);
    list.append("description.*");
    dir.setNameFilters(list);
    dir.setFilter(QDir::Files);
    qDebug() << dir.count();
    pbr->setRange(0, dir.count());
    pbr->setAlignment(Qt::AlignCenter);
    pbr->setMinimumWidth(200);
    pbr->move(20, 281);                                                                 // Костыли!!! (обход проблемы отображения прогресс-бара под MessageBox)
    pbr->show();

    QString newfolder = QDateTime::currentDateTime().toString("ddMMyyyy-HH.mm");
    QDir newdir("/mnt/");
    newdir.mkdir(newfolder);
    short count = 0;
    foreach (QString dirFile, dir.entryList()) {
        pbr->setValue(count++);
        pbr->update();
        success = QFile::copy(path + dirFile, "/mnt/" + newfolder + "/" + dirFile);
        if (!success)
            break;
    }
    pbr->hide();                                                                        // Прячем ProgressBar перед уничтожением ?  А для чего ? !!!
    delete pbr;
    return success;
}

long MainWindow::checkFreeSpace(QString mpoint)                                         // Функция определения объема свободного места в точке монтирования mpoint
{
    QProcess *df = new QProcess();
    df->start("df " + mpoint);
    df->waitForFinished();
    QString output = df->readAll();
    QStringList words = output.split(QRegExp(" "), QString::SkipEmptyParts);
    delete df;
    long kbfree = words.at(9).toLong();
    return kbfree;
}

bool MainWindow::waitForSending()                                                       // Слот цикла ожидания и обработки результатов передачи файлов в NextCloud
{
    bool allsended = false;                                                             // Флаг = true, если все файлы из списка были успешно переданы
    sendCloudOk = false;                                                                // Флаг = true, если все полностью передано в облако
    /* Принимаем от WebDAV сервиса список не переданных файлов (пустой список говорит об успешном завершении передачи) */
    /* и сигналы со счетчиком успешно переданных файлов для ProgressBar-а */
    QObject::connect(webdav, SIGNAL(curlTaskStatus(QStringList)), this, SLOT(getNotSendList(QStringList)));
    QObject::connect(webdav, SIGNAL(sendOk(short)), this, SLOT(waitProgress(short)));
    pdlg = new QProgressDialog(tr("Завершение передачи\n данных в сеть."),
                               tr("Отменить"), 0, sendcounter);                         // Берем общее число файлов, которые должны быть отправлены в NextCloud
    pdlg->setModal(true);
    pdlg->setMaximumWidth(220);
    pdlg->setMinimumDuration(500);                                                      // Первые 0,5 секунды не отрисовываем диалаго и ждем завершения передачи
    pdlg->setMinimum(0);
    pdlg->setAutoReset(true);
    pdlg->setAutoClose(true);
    pdlg->setWindowTitle(tr("Ожидайте.."));
    do
    {
        pdlg->setValue(0);                                                              // В цикле сбрасываем прогресс, стараемся закрыть все соединения и с задержкой
        emit cloudConnClose();                                                          // в надежде, что все было успешно передано открываем ProgressBarDialog
        pdlg->exec();                                                                   // для отображения того, что передать не успели. Ждем окончания завершения передачи по списку
        /* По достижении 100% (конца списка) закрываем ProgressBarDialog, провеяем результат и при необходимости идем на следующий круг */
    } while (!sendCloudOk && !pdlg->wasCanceled()) ;                                    // СДЕЛАТЬ ПО-ЧЕЛОВЕЧЕСКИ ПУТЕМ ПЕРЕДАЧИ СПИСКА В eventLoop (СДЕЛАТЬ ПОТОМКА QEventLoop)
    /* По случаю успешной передачи или отказа от нее, принимать сигналы от WebDAV нам более не требуется */
    QObject::disconnect(webdav, SIGNAL(curlTaskStatus(QStringList)), this, SLOT(getNotSendList(QStringList)));
    QObject::disconnect(webdav, SIGNAL(sendOk(short)), this, SLOT(waitProgress(short )));
    if (pdlg->value() == -1 && !pdlg->wasCanceled())
        allsended = true;                                                               // Считаем все файлы успешно переданными если ProgressBar закрылся по достижении 100%
    delete pdlg;
    return !allsended;                                                                  // Возвращаем 0 (false) в случае успешного завершения передачи
}

void MainWindow::waitProgress(short sended)                                             // Слот приёма числа успешно переданных в NextCloud файлов. Используется для ProgressDialogBar
{
    pdlg->setValue(sended);                                                             // Обновляем состояние ProgressBarDialog по мере поступления сигналов о переданных файлах
}

void MainWindow::getNotSendList(QStringList nsFilelist)                                 // Слот принимающий список файлов, которые не удалось передать
{
    if (!nsFilelist.isEmpty())
    {
        /* Если были ошибки передачи и список не пуст, обновляем количество итераций для достижения 100% ProgressBarDialog, размером списка файлов к передаче */
        pdlg->setMaximum(nsFilelist.size());
        sendCloudOk = false;                                                            // Сбросим признак успешности передачи в облако
        QMessageBox *qmsg = new QMessageBox(QMessageBox::Question,
                                            tr("Не удалось передать:") + QString::number(nsFilelist.count()),
                                            tr("Нажмите \"Yes\" чтобы попытаться еще\n" \
                                               "или \"No\" чтобы сохранить на съемный носитель"),
                                            QMessageBox::Yes | QMessageBox::No, 0);     // Сообщим об ошибке и предложим попробовать передать еще раз

        int answer = qmsg->exec();
        delete qmsg;
        if (answer == QMessageBox::Yes)
        {
            /* Получив согласие на повторную передачу, вновь открываем соединение с NextCloud и по списку отправляем запросы на передачу файлов */
            emit cloudConnInit(false);
            while (!nsFilelist.isEmpty())
            {
                emit sendToCloud(nsFilelist.first());
                nsFilelist.pop_front();
            }
        }
        else
        {
            /* В случае отказа от дальнейших попыток, выставляем признак успешности передачи в true и закрываем диалог ProgressBarDialog с выходом из цикла */
            sendCloudOk = true;
            pdlg->cancel();
            return;
        }
    }
    else
    {
        /* Если списко пришел пустым - все было успешно передано. Выставляем флаг успеха передачи и сбрасываем ProgressBarDialog */
        sendCloudOk = true;
        pdlg->reset();
    }
}

void MainWindow::onMainClicked()
{
    setCurrentIndex(MAINMENU);
}

void MainWindow::onSettingsClicked()
{
    setCurrentIndex(SETTINGS);
}

void MainWindow::pauseClicked()                                                         // Слот обработки нажатия паузы в опыте - ПОД ОБЪЕДИНЕНИЕ С MainWindow !!!
{
    task.pauseEnabled = !task.pauseEnabled;
}

void MainWindow::onNetSettingsClicked()                                                 // Слот вызова сетевых настроек - ПОД ОБЪЕДИНЕНИЕ С MainWindow !!!
{
    setCurrentIndex(NETWORK);
    netSettings.setCurrentParameters();
}

void MainWindow::onExperimentsClicked()                                                 // Слот вызова параметров опыта - ПОД ОБЪЕДИНЕНИЕ С MainWindow !!!
{
    setCurrentIndex(EXPERIMENTS);
}

void MainWindow::videoControlMode(bool control)                                         // Слот вызова окна видеоконтроля - ПОД ОБЪЕДИНЕНИЕ С MainWindow !!!
{
    if (control == true)
        calibratorWidget.showVideoViewer();
    else
        calibratorWidget.close();
}

void MainWindow::homed(bool success)                                                    // Слот обработки сигнала о завершении "Хоминга". Запускает по сигналу видеозахват и сеть
{
    if (success)
    {
        qDebug()<<"Homing success";
        if (!videoCapture->isRunning())
        {
            videoCapture->start();                                                      // ДОБАВИТЬ ЗАДЕРЖКУ К ЗАПУСКУ - СОЗДАЕТ ПРОБЛЕМЫ ПОТОКУ ВЫВОДА В TTYS1!!!!
            netSettings.initNetwork();                                                 // СДЕЛАТЬ ОТДЕЛЬНУЮ ПРОВЕРКУ РАБОТЫ И ЗАПУСКА СЕТИ ПОСЛЕ ХОМИНГА!!!
        }
    }
    else
        qDebug()<<"Homing fail";
}

void MainWindow::errard(int err)                                                        // Слот обработки ошибок от объекта управления контроллером исп.устройств. Писать в логи!!!
{
    switch (err) {
    case MODE:
        qDebug()<<"Mode select command error";
        break;
    case LIGHT:
        qDebug()<<"Light command error";
        break;
    case COOLER:
        qDebug()<<"Cooler command error";
        break;
    case MOTOR:
        qDebug()<<"Motor command error";
        break;
    case HEATER:
        qDebug()<<"Set heater temperature command error";
        break;
    case DATA:
        qDebug()<<"Send DATA command error";
        break;
    case TIMEOUT:
        qDebug()<<"Controller answer timeout";
        break;
    default:
        qDebug()<<"Unknow arduino error";
        break;
    }
}
