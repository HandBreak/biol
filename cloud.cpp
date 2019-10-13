#include <QDebug>
#include "cloud.h"

Cloud::Cloud()
{

}
Cloud::Cloud(const char *userlogin, \
             const char *password, \
             const char *cloudurl = "https://0.biol.cloud:8080/remote.php/webdav/")
{
    usr = userlogin;
    pwd = password;
    url = cloudurl;
    curl_handle = NULL;
}

Cloud::~Cloud()
{

}

#define READ_3RD_ARG size_t
static curlioerr my_ioctl(CURL *handle, curliocmd cmd, void *userp)                     // Обработка ошибок ввода/вывода библиотеки cURL (взято из премера использования)
{
  int *fdp = (int *)userp;
  int fd = *fdp;

  (void)handle; /* not used in here */

  switch(cmd) {
  case CURLIOCMD_RESTARTREAD:
    /* mr libcurl kindly asks as to rewind the read data stream to start */
    if(-1 == lseek(fd, 0, SEEK_SET))
      /* couldn't rewind */
      return CURLIOE_FAILRESTART;

    break;

  default: /* ignore unknown commands */
    return CURLIOE_UNKNOWNCMD;
  }
  return CURLIOE_OK; /* success! */
}

/* read callback function, fread() look alike */
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  ssize_t retcode;
  curl_off_t nread;

  int *fdp = (int *)stream;
  int fd = *fdp;

  retcode = read(fd, ptr, (READ_3RD_ARG)(size * nmemb));

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
          " bytes from file\n", nread);

  return retcode;
}

bool Cloud::curlOpenConn(bool newfolder)
{
    notsended = QStringList();
    if(!curl_handle)
    {
        curl_handle = curl_easy_init();
        sended = 0;
    }
    if(curl_handle && newfolder)
    {
        curlWebCreate();
        curl_handle = curl_easy_init();
    }
    return 0;
}


bool Cloud::curlSendFile(QString filename)
{
    long errcode;                                                                       // Код ошибки от HTTP сервера. Сделать его возврат в вызывающую функцию!!!
    for (short r = 1; r <= retrains; r++)                                               // Выполняем в пределах допустимого числа повторных попыток передачи файла
    {
        if (curlSend(errcode, filename) == 0)                                           // Если выход из функции отправки завершился без ошибки
        {
            qDebug() << "Error code# " << errcode;
            if (errcode == 201 || errcode == 204 || errcode == 100)                     // Если статус Created, Overwrited or Continue
            {
                sended++;                                                               // Увеличить счетчик переданных файлов
                emit sendOk(sended);                                                    // Послать число успешно переданных файлов
                break;                                                                  // Прервать цикл загрузки файла
            }
            qDebug() << "Retrain# " << r;
        }
        else
            qDebug() << "Error status != 0";

        if (r == retrains)
            notsended.append(filename);                                                 // Если попытки исчерпаны, добавить отправляемый файл в список не отправленных
    }
    if (notsended.isEmpty())
    {
        if (filename.contains("description"))                                           // Если неотправленных не осталось, можно переименовать файл флаг-дескриптор
            curlWebRaname(errcode, filename);                                           // Надо обрабатывать ошибки !!!
        return 0;                                                                       // Пока считаем, что переименование всегда успешно завершилось
    }
    else
        return true;                                                                    // Но если есть не переданные,  возвращаем 1
}

int Cloud::curlSend(long &errcode, QString file)                                        // Отправка файла. На входе полное имя (с путём) и адрес переменной, куда будет сохранен ответ сервера
{
    if(curl_handle)                                                                     // Выполняем только, если подключение уже было открыто
        {
            int hd;
            struct stat file_info;
            hd = open(file.toLocal8Bit().constData(), O_RDONLY);
            fstat(hd, &file_info);
            QString targeturl = url + QString(date + "/" + time + "/") + file.section('/', -1);
            /* which file to upload */
            curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void *)&hd);
            /* pass the file descriptor to the ioctl callback as well */
            curl_easy_setopt(curl_handle, CURLOPT_IOCTLDATA, (void *)&hd);
//             curl_easy_setopt(curl_handle, CURLOPT_STDERR, logfile);
            /* HTTPs  Запрос */
            curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
            /* set the ioctl function */
            curl_easy_setopt(curl_handle, CURLOPT_IOCTLFUNCTION, my_ioctl);
            /* enable "uploading" (which means PUT when doing HTTP) */
            curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
            /* HTTP PUT please */
            curl_easy_setopt(curl_handle, CURLOPT_PUT, 1L);
            /* дополнительные опции CURL устновка таймаутра 10 сек. вместо бесконечности (300сек) */
            curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, timeout);
            // отключим сигналы - могут вызывать сбой при таймауте
            curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
            /* enable verbose for easier tracing */
             curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
            // не проверять SSL сертификат
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
            // не проверять Host SSL сертификата
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
            /* and give the size of the upload, this supports large file sizes
               on systems that have general support for it */
            curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE,
                             (curl_off_t)file_info.st_size);
            /* set user name and password for the authentication */
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, getAuthStr().toLocal8Bit().constData());
            curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
            curl_easy_setopt(curl_handle, CURLOPT_URL, targeturl.toLocal8Bit().constData());

            CURLcode res = curl_easy_perform(curl_handle);
            if(res == CURLE_OK)
                 curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &errcode);      // Ответ сервера будет возвращен по адресу переданному в функцию
            else
               // qDebug() <<"Error #"<<res<<" "<<curl_easy_strerror(res) <<endl;
            close(hd); /* close the local file */
            return (int)res;                                                            // Код ошибки функции libCURL
        }
    //qDebug()<<"Done!";
    return -1;                                                                          // Если соединение не было установлено ранее, возвращаем код ошибки = -1
}

int Cloud::curlWebRaname(long &errcode, QString file)                                   // Выполним переименования файла флага-дескриптора
{
    if(curl_handle)
    {
        QString targeturl = url + QString(date + "/" + time + "/") + file.section('/', -1);
        QString destinurl = QString("Destination: ") + targeturl.section(QRegExp("^(https://([^/]+))"), -1);
        destinurl.truncate(destinurl.lastIndexOf(QChar('.')) + 1);
        destinurl.append("txt");

        curl_slist *slist = NULL;
        slist = curl_slist_append(slist, destinurl.toLocal8Bit().constData());
        curl_easy_setopt(curl_handle, CURLOPT_USERPWD, getAuthStr().toLocal8Bit().constData());
        curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
        curl_easy_setopt(curl_handle, CURLOPT_URL, targeturl.toLocal8Bit().constData());
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "MOVE");
        curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, slist);
        CURLcode res = curl_easy_perform(curl_handle);
        if(res == CURLE_OK)
             curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &errcode);          // Ответ сервера будет возвращен по адресу переданному в функцию
        return (int)res;
    }
    return -1;                                                                          // Если соединение не было установлено ранее, возвращаем код ошибки = -1
}

int Cloud::curWebMkDir(long &errcode, QString path, QString usrpwd)
{
    if(curl_handle)
    {
        QString targeturl = url + path;
        /* дополнительные опции CURL устновка таймаутра 10 сек. вместо бесконечности (300сек) */
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_USERPWD, usrpwd.toLocal8Bit().constData());
        curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
        curl_easy_setopt(curl_handle, CURLOPT_URL, targeturl.toLocal8Bit().constData());
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "MKCOL");
        CURLcode res = curl_easy_perform(curl_handle);
        if(res == CURLE_OK)
             curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &errcode);          // Ответ сервера будет возвращен по адресу переданному в функцию
        return (int)res;
    }
    return -1;                                                                          // Если соединение не было установлено ранее, возвращаем код ошибки = -1
}

bool Cloud::curlWebCreate()                                                             // Получим текущие дату, время и создадим соответствующую структуру подкаталогов
{                                                                                       // Верхний уровень - дата эксперимента, нижний - время начала
    long errcode;
    date = QDate::currentDate().toString("ddMMyyyy");
    curWebMkDir(errcode, date, getAuthStr());
    time = QTime::currentTime().toString("HHmmss");
    QString path = QString(date + "/" + time);
    curWebMkDir(errcode, path, getAuthStr());
    curl_easy_cleanup(curl_handle);
    return true;                                                                        // Всегда возвращает true !!!  Решить вопрос с обработкой ошибок !!!
}

QString Cloud::getAuthStr()
{
    QString usrpwd = usr + QString(":") + pwd;
    return usrpwd;
}

bool Cloud::curlCloseConn()                                                             // Закрыть соединение, если таковое имеется
{
    if(curl_handle)
    {
        curl_easy_cleanup(curl_handle);
        curl_handle = NULL;
    }

    emit curlTaskStatus(notsended);                                                     // Сообщить список (число) файлов, которые не удалось отправить
    notsended.clear();
    return 0;
}
