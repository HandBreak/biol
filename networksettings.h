#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H
/*
 * Класс реализующий интерфейс для настройки сетевых параметров и инициализации сетевого
 * подключения. Для использования требует подключения библиотеки работы с JSON-объектами.
 * Класс взаимодействует с ОС средствими uci-интерфейса OpenWRT
 *
 * TODO:    Перенести встроенный виджет пользовательского интерфейса управления сетевыми
 *          настройками на страницу стекового виджета MainWindow и привести элементы уп-
 *          равления к общему touchscreen-ориентированному стилю.
 *          Решить проблему с периодическими ошибками инициализации сети.
 *          Реализовать интерфейс ввода статического IP-адреса, маски, шлюза.
 *          Реализовать интерфейс настройки точки доступа
 *          Реализовать настройку параметров подключения к облачному сервису
 */
#include <QWidget>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QProcess>
#include <QByteArray>

#include <../qjson-backport/qjsondocument.h>
#include <../qjson-backport/qjsonobject.h>
#include <../qjson-backport/qjsonarray.h>

namespace Ui {
class NetworkSettings;
}

class NetworkSettings : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkSettings(QWidget *parent = 0);
    ~NetworkSettings();
    void setCurrentParameters();                                                        // Заполняет соответствующие поля текущими значениями. Вызывается из MainWindow после инициализации сети
    void initNetwork();                                                                 // Функция инициализация сети. Вызывает системную функцию 'network start'
    QString getLanIp();

private:
    Ui::NetworkSettings *ui;                                                            // Встроенный интерфейс управления настройками
    QProcess *uci;                                                                      // Указатель на процесс в котором вызываются функции uci операционной системы

    QStringList readLanSettings();                                                      // Функция считывания настроек проводной сети
    QStringList readWiFiSettings();                                                     // Функция считывания настроек беспроводной сети
    QString getInputMask(ushort);                                                       // Функция преобразования 32-х битной сетевой маски в текстовое значения вида aaa.bbb.ccc.ddd
    QString getInputIpAddr(\
                          QLineEdit *, \
                          QLineEdit *, \
                          QLineEdit *, \
                          QLineEdit *);                                                 // Функция формирования текствой формы ip адреса вида aaa.bbb.ccc.ddd на основании содержимого полей ввода
    void readSettings();                                                                // Функция считывания всех сетевых параметров (вызывает чтение LAN и WiFi параметров)
    char setInputMask(QString );                                                        // Функция преобразования сетевой маски текстового формата вида aaa.bbb.ccc.ddd в 32-х битное значение
    void fillIpAddrFields(const QString, \
                          QLineEdit *, \
                          QLineEdit *, \
                          QLineEdit *, \
                          QLineEdit *);                                                 // Функция разбиения тестового формата IP адреса вида aaa.bbb.ccc.dd на соответствующие заполненные октеты

signals:
    void toMainReturn();                                                                // Сигнал выхода в главное меню (после переноса в основной MainWindow-виджет будет удалён !!!)

private slots:
    void onReturnClicked();                                                             // Обработка реакции на нажатие "Выход"
    void onAcceptClicked();                                                             // Обработка реакции на нажатие "Применить"
};

#endif // NETWORKSETTINGS_H
