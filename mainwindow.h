#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/*
 * Класс реализующий управляющий интерфейс прибора и общую логику поведения.
 *
 * TODO:    Полностью переделать интерфейс, объединив в StackedWidget интерфейсы из всех
 *          классов, включая videowidget, networksettings, виджет настроек и управления
 *          экспериментом.
 *          Очистить от лишних функций и коннектов.
 *          Предусмотреть сохранение пресетов в памяти устройства с последующим выбором
 *          предустановленного варианта.
 *          Переделать аутентификацию в NextCloud. Логин формировать из MAC-адреса, пароль
 *          генерировать по своему алгоритму, аналогичному тому, что будет сделан в NextCloud
 *          Предусмотреть возможность изменения URL-хранилища.
 */
#include <QProcess>
#include <QProgressBar>
#include <QProgressDialog>
//#include <QMainWindow>
#include <QStackedWidget>
#include <QTextStream>
#include <QMessageBox>
#include <QEventLoop>
#include <QRegExp>
#include <QThread>
#include <QTimer>
#include <QDir>

#include <videowidget.h>
#include <capturethread.h>
#include <networksettings.h>
#include <calibratorwidget.h>
#include <task.h>
#include <cloud.h>
#include <arduino.h>
#include <termostat.h>
#include <experiments.h>
#include <taskexecutor.h>
#include <actuatorinterface.h>
#include <actuatorconstants.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QStackedWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Task *task;                                                                         // Указатель на задание для эксперимента

private:
    Ui::MainWindow *ui;
    ActuatorInterface actInterface;                                                     // Объект класса приёмопередатчика команд контроллеру исполнительных устройств (отдельный поток!)
    Arduino *arduino;                                                                   // Указатель на объект класса управления контроллером исполнительных устройств (отдельный поток!)
    TaskExecutor *taskExecutor;                                                         // Указатель на объект класса исполнителя задания эксперимента (отдельный поток!)
    Termostat *termostat;                                                               // Указатель на объект класса термостата (отдельный поток!)
    Experiments *oExperiments;                                                          // Указатель на объект класса формирования задания для эксперимента. (ПЕРЕНЕСТИ в MainWindow и создать интерфейс!!!)
    NetworkSettings *netSettings = NULL;                                                // Указатель на объект класса конфигуратора сети. (ПЕРЕНЕСТИ в MainWindow интерфейс!!!)
    CalibratorWidget *calibratorWidget;                                                 // Указатель на объект класса виджета калибратора. (ПЕРЕНЕСТИ в MainWindow интерфейс!!!)
    QProgressDialog *pdlg;                                                              // Указатель на объект командной строки для вызова функций операционной системы
    CaptureThread *videoCapture;                                                        // Указатель на объект видеозахвата
    VideoWidget *videoWidget;                                                           // Указатель на видеовиджет
    QThread *taskExecutorThread;                                                        // Указатель на поток для исполнителя задания эксперимента
    QThread *captureThread;                                                             // Указатель на поток объекта видеозахвата
    QThread *webdavThread;                                                              // Указатель на поток загрузки в NextCloud
    QThread *termoThread;                                                               // Указатель на поток термостабилизатора
    QThread *arduinoThread;                                                             // Указатель на поток управления контроллером
    QTimer tsTimer;                                                                     // Таймер для циклической активации термостабилизатора во время эксперимента
    Cloud *webdav;                                                                      // Указатель на объект класса взаимодействующего с сервисом NextClowd (отдельный поток!)
    QString mac;                                                                        // Уникальный идентификатор (mac-адрес сетевого адаптера прибора)
    QString path;                                                                       // Путь хранения снимков
    QFile filelist;                                                                     // Файл флага-дескриптора всех снимков эксперимента
    qint64 dsize;                                                                       // Размер хранилища
    short sendcounter;                                                                  // Счетчик числа переданных в NextCloud файлов
    short shotcounter;                                                                  // Счетчик сделанных снимков
    bool sendCloudOk;                                                                   // Флаг состояния передачи снимка в NextCloud (успешно / с ошибкой)
    bool waitForSending();                                                              // Цикл ожидания передачи данных в сеть
    bool driveMount();                                                                  // Функция подключения внешнего носителя. Возвращает true, если удалось.
    bool driveUmount();                                                                 // Функция отключения внешнего носителя. Возвращает true, если удалось.
    bool saveToRemovable(bool);                                                         // Функция записи на съемный носитель.  Возвращает true, если удалось. На входе флаг "Должно быть сохранено!"
    bool cpToRemovableDrive();                                                          // Функция копирования на съемный носитель
    long checkFreeSpace(QString);                                                       // Функция проверки свободного места. На входе точка монтирования носителя

public slots:
    void onShotSignal(QStringList);                                                     // По сигналу от TaskExecutor, получив список параметров съемки, запрашивает кадр у CaptureThread и сохраняет его
    void onExperimentInProgress(bool);                                                  // По началу и завершению эксперимента создаёт каталоги и файл описания или переименовывает и отправляет его в NextCloud
    void waitProgress(short);                                                           // Получает число успешно переданных файлов для обновления ProgressBar
    void getNotSendList(QStringList);                                                   // Принимает список файлов, которые не удалось передать, запрашивает реакцию пользователя, отправляя запрос на повторную передачу
    void pauseClicked();                                                                // При нажатии "Паузы" инвертирует соответствующий флаг в задании (Task) для приостановки/продолжения эксперимента
    void onNetSettingsClicked();                                                        // По нажатию кнопки вызывает интерфейс сетевых настроек (ПЕРЕНЕСТИ в MainWindow интерфейс!!!)
    void onExperimentsClicked();                                                        // По нажатию кнопки вызывает интерфейс свойств эксперимента (ПЕРЕНЕСТИ в MainWindow интерфейс!!!)
    void videoControlMode(bool);                                                        // Включает / отключает отображение виджета видеоконтроля в процессе эксперимента
    void homed(bool);                                                                   // По завершению 'Homing' запускает инициализацию Видеокамеры и Сетевых интерфейсов. (параллельный процесс может вызывать сбои)
    void errard(int);                                                                   // Получает и обрабатывает ошибки от объекта управления контроллером исп.устройств (Arduino)

    void onMainClicked();                                                               // Вызывает основное меню
    void onSettingsClicked();                                                           // Вызывает меню настроек

signals:
    void doHoming();                                                                    // Вызывает 'Homing' при запуске программного обеспечения
    void lightOff();                                                                    // Гасит подстветку при запуске ПО
    void coolerOff();                                                                   // Отключает вентиляцию при запуске ПО
    void heaterOff();                                                                   // Отключает подогрев при запуске ПО
    void cloudConnInit(bool);                                                           // Запрашивает открытие соединения с NextCloud
    void sendToCloud(QString);                                                          // Запрашивает отправку файла в NextCloud
    void cloudConnClose();                                                              // Запрашивает закрытие подключения к NextCloud
};

#endif // MAINWINDOW_H
