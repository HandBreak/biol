#ifndef CLOUD_H
#define CLOUD_H
/*
 * Класс реализующий функции аутентификации и передачи снимков в облачный сервис NextCloud.
 * На входе получает параметры аутентификации и адрес хранилища снимков. При отсутствии
 * таковых использует дефолтный адрес хранилища, но укаталеи на логин и пароль остаются
 * не инициализированы. При отсутствии входных параметров вообще, все указатели на переменные
 * параметров подключения остаются не инициализированными !!!
 * Должен исполняться в отдельном потоке!
 *
 * TODO:    Сделать полноценную обработку ошибок соединения и передачи. В том числе для
 *          функции curlWebCreate
 *          Сделать корректную инициализацию параметров подключения в конструкторе ???
 */
#include <QObject>
#include <QDate>
#include <QTime>
#include <QString>
#include <QStringList>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <curl.h>
#include <curlver.h>
#include <sys/stat.h>

const short retrains = 3;                                                               // Количество попыток передачи файлов
const int   timeout  = 10;                                                              // Таймаут попытки (сек)

class Cloud : public QObject
{
    Q_OBJECT

public:
    Cloud();
    Cloud(const char *userlogin,\
          const char *password, \
          const char *cloudurl );
    ~Cloud();

private:
    const char *usr;                                                                    // Имя пользователя
    const char *pwd;                                                                    // Пароль полдьзователя
    const char *url;                                                                    // Адрес хранилища
    bool curlWebCreate();                                                               // Функция создания нового опыта в "облаке". Создаёт структуру каталогов (дата/время). Всегда возвращает success !!!
    short sended;                                                                       // Счетчик числа успешно отправленных файлов (для индикатора прогресса)
    int curlSend(long &errcode, QString file);                                          // Функция отправки локального файла 'file' в облако. Возвращает код ошибки или -1, если соединение не установлено
    int curWebMkDir(long &errcode, QString path, QString usrpwd);                       // Функция создаёт требуемый подкаталог. На входе кроме полного пути, получает login:password
    int curlWebRaname(long &errcode, QString file);                                     // Функция переименования файла флага-дескриптора после завершения загрузки всего. Заменяет расширение 'file' на .txt
    QString getAuthStr();                                                               // Возвращает строку типа 'user:password' из текущих значений usr и pwd
    QString date;                                                                       // Хранит дату начала опыта (и соответствующее имя подкаталога)
    QString time;                                                                       // Хранит время начала опыта (и соответствующее имя подкаталога)
    QStringList notsended;                                                              // Список файлов, которые не удалось загрузить на сервер
    CURL *curl_handle;                                                                  // Дескриптор объекта cURL
    // FILE *logfile;                                                                   // Регистрация событий в Log-файл

signals:
    void curlTaskStatus(QStringList notsended);                                         // Отправляет список не переданных позле закрытия соедниения файлов в интерфейс пользователя
    void sendOk(short sended);                                                          // Отправляет количество успешно переданных файлов для индикатора прогресса

public slots:
    bool curlOpenConn(bool newfolder);                                                  // Открывает соединение, или создаёт структуру подкаталогов, если соединение было открыто ранее и определено имя каталога
    bool curlCloseConn();                                                               // Закрывает соединение, посылает список непереданных файлов для повторного запроса, а затем очищает его.
    bool curlSendFile(QString);                                                         // Осуществляет попытки передать заданный файл, обрабатывает ошибки передачи, заполняет список не переданных
};                                                                                      // и вызывает переименование 'description.id', если все полностью передано

#endif // CLOUD_H
