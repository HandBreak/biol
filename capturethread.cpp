#include "capturethread.h"
#include <QDebug>

CaptureThread::CaptureThread(VideoWidget *parent) :
    QThread(parent)
{
// Конструктор передаёт родителю  какой-то указатель ????
// объявляет переменную capture как false
// и устанавливает переменную файлового дескриптора в -1 (ошибка открытия файла)

    this->videovidget=(VideoWidget*)parent;
    capture=false;
    ctrenum=false;
    takeshot=false;
    sprintf(xrstr,"%s","");
    sprintf(yrstr,"%s","");
    fd = -1;
}

void CaptureThread::run(){
// Запускаем цикл захвата
fd = -1;

// Задаём устройство видеозахвата (dev_name = /dev/video0) и пробуем его открыть. 'fd' - дескриптор открытого файла
// Если попытка открытия приводит к ошибке (код = -1), выводим сообщение ("не удалось открыть устройство" и выходим из функции)
    fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
           qDebug("Cannot open device");
           //exit(EXIT_FAILURE);
           return;
    }

    if (strlen(xrstr) == 0 || strlen(yrstr) == 0)
        setResolution(3);                                                               // Устанавливаем разрешение 640x480, если ранее не было установлено
    xres = std::atoi(xrstr);
    yres = std::atoi(yrstr);

    enumerate_menu();                                                                   // Читаем параметры в вектор после обнаружения камеры

// Объявляем указатель на статическую структуру данных для преобразования данных
// Объявляем статическую структуру 'src_fmt', описывающую формат данных видеоисточника
// Объявляем указатель на приёмный буфер
    static struct v4lconvert_data *v4lconvert_data;
    static struct v4l2_format src_fmt;
    static unsigned char *dst_buf;

// (CLEAR(fmt) - макрос, выполняющий заполнение области памяти (буфера), на которую указывает аргумент, размером указанным в аргументе, кодом #00)
    CLEAR(fmt);
// Создаём в очищенном буфере структуру, описывающую формат данных.  Область .type                  содержит описатель типа структуры (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
//                                                                           .fmt.pix.width         содержит требуемое значение количества точек по горизонтали (int)
//                                                                           .fmt.pix.height        содержит требуемое значение количества точек по вертикали (int)
//                                                                           .fmt.pix.pixelformat   содержит описатель формата кодирования цвета пиксела (V4L2_PIX_FMT_RGB24 =  RGB 8-8-8 + 3 - 4-х байтовый)
//                                                                           .fmt.pix.field         содержит описатель режима чередования полей (V4L2_FIELD_INTERLACED = 4) (int)
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = xres;
    fmt.fmt.pix.height      = yres;


    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);                                                     // Осущствляем вызов xioctl, в котором передаём 1:Дескриптор открытого файла, 2:тип запроса (VIDIOC_S_FMT - установка формата), 3:ссылка на буфер структуры описателя формата 'fmt'
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {                                // Сверяем возвращенный драйвером формат '.fmt.pix.pixelformat' с выбранным нами ранее.
           printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");               // Если получили формат отличный от RGB24, вылетаем с ошибкой
           //exit(EXIT_FAILURE);
           return;
    }
    if ((fmt.fmt.pix.width != xres) || (fmt.fmt.pix.height != yres))                    // Сверяем возвращенные разрешения по вертикали и горизонтали '.fmt.pix.width' и '.fmt.pix.height'
           printf("Warning: driver is sending image at %dx%d\n",                        // Если хоть один из них отличается от заданного в структурой 'fmt' 640x480, предупреждаем, что драйвер передает изображение в
                   fmt.fmt.pix.width, fmt.fmt.pix.height);                              // формате, полученном соответствющими элементами структуры 'fmt'

// Структуру, на которую указывает 'v4lconvert_data' заполняем выводом функции 'v4lconvert_create', получайющей в качестве аргумента дескриптор файла видеоисточника
    v4lconvert_data = v4lconvert_create(fd);
    if (v4lconvert_data == NULL)                                                        // Если получаем в ответ NULL, то должно быть выведено в консоль отладки 'v4lconvert_create'
        qDebug("v4lconvert_create");
    if (v4lconvert_try_format(v4lconvert_data, &fmt, &src_fmt) != 0)                    // Далее, если функция 'v4lconvert_try_format', получающая на входе указатель на структуру 'v4lconvert_data', ссылку на структуру описателя формата 'fmt', ссылку на структуру формата источниа 'src_fmt'
        qDebug("v4lconvert_try_format");                                                // возвращает значение отличное от нуля,  сообщаем о том, что v4lconvert_try_format (пытаемся преобразовать формат)
    xioctl(fd, VIDIOC_S_FMT, &src_fmt);                                                 // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор открытого файла, 2:тип запроса (VIDIOC_S_FMT - установка формата), 3:ссылка на буфер структуры описателя формата источника 'src_fmt'
    dst_buf = (unsigned char*)malloc(fmt.fmt.pix.sizeimage);                            // Переменной 'dst_buf' (приёмный буфер) выделяем память, размер которой задаётся параметром '.fmt.pix.sizeimage', таким образом dst_buf становится указателем на область данных типа беззнаковые символы (байт)

// (CLEAR(reg) - макрос, выполняющий очистку (заполнение кодом #00) области памяти (буфера), на которую ссылается reg, размером reg,  где reg - запрашиваемый v4l буфер
    CLEAR(req);
    req.count = 2;                                                                      // .count                сообщаем, что хотим получить у драйвера 2 (два) буфера
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                             // .type                 в поле буфера 'тип', задаём его описатель (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    req.memory = V4L2_MEMORY_MMAP;                                                      // .memory               в поле буфера 'память' задаём параметр (V4L2_MEMORY_MMAP = 1)
    xioctl(fd, VIDIOC_REQBUFS, &req);                                                   // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_REQBUFS - выделение буферов), 3:ссылка на буфер структуры запроса reg

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));                             // Переменной 'buffers' (выделяемые буферы), присваиваем область инициализированной (очищенной) памяти для двух элементов, размер которых, равен буферу ????
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {                           // Пока индекс (номер) буфера = 0 и меньше числа запрошенных буферов, увеличиваем номер буфера на 1)
           CLEAR(buf);                                                                  // Выполняем макрос очистки буфра, на который ссылается 'buf', размером с 'buf' (Объявлен в заголовочном файле как структура 'v4l2_buffer')

           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;                               // .type                 в поле буфера 'тип', задаём его описатель (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
           buf.memory      = V4L2_MEMORY_MMAP;                                          // .memory               в поле буфера 'память' задаём параметр (V4L2_MEMORY_MMAP = 1)
           buf.index       = n_buffers;                                                 // .index                в поле буфера 'номер' (индекс), задаем текущее значение номера буфера n_buffers

           xioctl(fd, VIDIOC_QUERYBUF, &buf);                                           // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_REQBUFS - выделение буферов), 3:ссылка на буфер структуры запроса buf

           buffers[n_buffers].length = buf.length;                                      // В поле 'lenght' n-ного элемента массива 'buffers', заносим значение размера буфера 'buf'
           buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,                       // В поле 'start' n-ного элемента массива 'buffers', заносим результат вызова функции 'v4l2_mmap', которая на входе получает NULL, длину буфера, режимы, дескриптор файла, смещение в буфере
                         PROT_READ | PROT_WRITE, MAP_SHARED,
                         fd, buf.m.offset);

           if (MAP_FAILED == buffers[n_buffers].start) {                                // Если поле 'start' текущего элемента из массива буферов содержит (-1 - MAP_FAILED), вылетаем с ошибкой выделения памяти
                   qDebug("mmap");
                   //exit(EXIT_FAILURE);
                   return;
           }
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {                                      // Пока i=0 и меньше номера актуального буфера,  увеличиваем итератор
           CLEAR(buf);                                                                  // Выполняем макрос очистки буфера, на который ссылается 'buf', размером с 'buf' (Объявлен в заголовочном файле как структура 'v4l2_buffer')
           buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                      // .type                 в поле буфера 'тип', задаём его описатель (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
           buf.memory = V4L2_MEMORY_MMAP;                                               // .memory               в поле буфера 'память' задаём параметр (V4L2_MEMORY_MMAP = 1)
           buf.index = i;                                                               // .index                в поле буфера 'номер' (индекс), задаем текущее значение итератора
           xioctl(fd, VIDIOC_QBUF, &buf);                                               // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_QBUF - ставим буферы в очередь), 3:ссылка на буфер структуры запроса 'buf'
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                                 // Переменной 'type' присваиваем (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    xioctl(fd, VIDIOC_STREAMON, &type);                                                 // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_STREAMON - включаем камеру), 3:ссылка переменную, хранящую тип буфера ( V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)

// Начинаем цикл накопления данных,  инициализируем переменную di,  объявляем символьный массив header[], содержащий текстовую строку с параметрами разрешения и чего-то ещё ???
    int di=0;
    bool saveframe;
    char header[20]="P6\n";

// Блок переменных для экспериментального блока обрезки кадра
    __u8  pixsize     = 3;
    __u16 wcropsize   = yres;
    __u16 hcropsize   = yres;
    __u16 wcropshift  = (fmt.fmt.pix.width - wcropsize) / 2;                           // Центрирование кадра
    __u16 hcropshift  = (fmt.fmt.pix.height - hcropsize) / 2;                          // Центрирование кадра;
    __u16 rowsize     = wcropsize * pixsize;
    __u32 framesize   = hcropsize * rowsize;
    __u32 skiprows    = fmt.fmt.pix.width * pixsize;
    __u32 skipinrow   = wcropshift * pixsize;
    strcat(header, yrstr);
// Конец блока


//    strcat(header, xrstr);
    strcat(header, " ");
    strcat(header, yrstr);
    strcat(header, " 255\n");
    while(capture){                                                                     // Цикл накопления выполняем до тех пор, пока capture = true (в конструкторе объявляется как false)
        saveframe = takeshot;                                                           // Фиксируем состояние флага "сделать снимок" непосредственно перед захватом кадра
        do {
                FD_ZERO(&fds);                                                          // Макрос FD_ZERO() передаёт ссылку на 'fds' в макрос __FD_ZERO()
                FD_SET(fd, &fds);                                                       // Макрос FD_SET() передаёт имя файла и ссылку на 'fds' аналогичным образом  (fds - объект типа fd_set)

                /* Timeout. */
                tv.tv_sec = 2;                                                          // .tv_sec               временной интервал = 2 секундам (переменная типа __time_t)
                tv.tv_usec = 0;                                                         // .tv_usec              временной интервал = 0 микросекундам (переменная типа __suseconds_t)

                r = select(fd + 1, &fds, NULL, NULL, &tv);                              // r присваиваем int результат выполнения функции 'select', получающей на входе  1:дескриптор фала + 1, 2:Ссылку на 'fds', NULL, NULL, ссылку на структуру типа 'timeval' (описана в заголовочном файле)

        } while ((r == -1 && (errno = EINTR)));                                         // Ожидаем пока наша r содержит код ошибки (-1) и системный вызов (прерывание) возвращает "4"
        if (r == -1) {                                                                  // Если по завершению прерывания (EINTR) r всё равно содержит код ошибки (-1), отправляем в отладочную консоль сообщение 'select' и выходим с ошибкой (1)
                qDebug("select");
                //exit(1) ;
                return;
        }

// Выполняем макрос очистки буфера, на который ссылается 'buf', размером с 'buf' (Объявлен в заголовочном файле как структура 'v4l2_buffer')
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                         // .type                 в поле буфера 'тип', задаём его описатель (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
        buf.memory = V4L2_MEMORY_MMAP;                                                  // .memory               в поле буфера 'память' задаём параметр (V4L2_MEMORY_MMAP = 1)
        xioctl(fd, VIDIOC_DQBUF, &buf);                                                 // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_QBUF - ставим буферы в очередь), 3:ссылка на буфер структуры запроса 'buf'

        try{                                                                            // Пробуем...
            if (v4lconvert_convert(v4lconvert_data,                                         // Если функция преобразования 'v4lconvert_convert',
                                &src_fmt,                                               // получившая на входе указатель на буфер структуры формата источника 'src_fmt',
                                &fmt,                                                   // буфер структуры формата 'fmt',  указатель на начало актуального буфера в массиве, число сохранённых в буфере байт,
                                (unsigned char*)buffers[buf.index].start, buf.bytesused,
                                dst_buf, fmt.fmt.pix.sizeimage) < 0) {                  // указатель на адрес приёмного буфера, '.fmt.pix.sizeimage' - размер изображения, возвращает код ошибки (-1), тогда
                if (errno != EAGAIN)                                                    // если код ошибки отличается от (11 = TRY AGAIN),  выводим в отладочную консоль 'v4l_convert'
                        qDebug("v4l_convert");
            }

/*      Универсальный цикл захвата кадра. Содержит избыточное копирование кадра
 *
        unsigned char* frame=(unsigned char*)malloc(fmt.fmt.pix.sizeimage+qstrlen(header));
        memmove(frame+qstrlen(header), dst_buf, fmt.fmt.pix.sizeimage);                  // Перемещаем блок в область, на которую указывает 'frame' со смещением на длину текстовой строки,  хранимой в массиве 'header' ( = P6\n640 480 255\n), из буфера конвертора 'dst_buf', длиной сторки в header
        memcpy(frame,header,qstrlen(header));                                            // Копируем (без промежуточного буфера) блок в область, на которую указывает 'frame', из области на начало которой указывает 'header', длиной строки в header

        QImage qq;//=new QImage(dst_buf,640,480,QImage::Format_RGB32);                   // Создаём в динамической памяти объект 'qq' типа QImage, каким-то образом получающий на входе указатель на 'dst_buf', разрешение по X, разрешение по Y, признак RGB32 формата данных)
        if(qq.loadFromData(frame,fmt.fmt.pix.sizeimage+qstrlen(header),"PPM")){
            if(videovidget->isVisible()){                                                // Если предок (виджет видеоокна) имеет сатус "Видимый (isVisible),
                videovidget->img=qq.copy((fmt.fmt.pix.width - fmt.fmt.pix.height)/2,\
                                     0, fmt.fmt.pix.height, fmt.fmt.pix.height);         // Копируем в видеовиджет только центр кадра (квадрат по меньшей стороне)

                videovidget->update();                                                   // Вызываем у предка (видеовиджета) метод 'update', который перерисовывает изображение
              //this->msleep(50);                                                        // Ожидаем 50 миллисекунд (закомментировано)
            }
            if (saveframe)                                                               // Если есть требование сохранить снимок,
            {
                cs=qq.copy((fmt.fmt.pix.width - fmt.fmt.pix.height)/2,\
                           0, fmt.fmt.pix.height, fmt.fmt.pix.height);

                cs=videovidget->img;
                takeshot = false;                                                        // Сбросим требование сохранения снимка
            }
        //qApp->processEvents();                                                         // Иначе отправляем qApp сигнал о событии (закомментировано)
        }
        if(frame)                                                                        // Если указатель 'frame' true, освобождаем память выделенную под 'frame', но сам указатель при этом не меняется, то есть указывает на тот же блок памяти, а не на NULL
            free(frame);
        if (!takeshot)
            msleep(40);                                                                  // Ограничиваем скорость захвата 25к/сек (40 мсек/кадр) ДОБАВЛЕНО 050419!
*
*/

//   Здесь вставляем экспериментальный блок обрезки кадра перед копированием в кадровый буфер
            __u16 nl = 0;
            unsigned char* cframe=(unsigned char*)malloc(framesize + qstrlen(header));
            for (__u16 l = hcropshift; l < hcropshift + hcropsize; l++)
            {
                memcpy(cframe+qstrlen(header) + nl * rowsize,\
                       dst_buf + skipinrow + l * skiprows,\
                       rowsize);
                nl++;
            }
            memcpy(cframe,header,qstrlen(header));
//  Конец блока обрезки

// Собственно загрузка данных в объект производится строкой ниже (в проверке условия), с помощью Qt метода loadFromData, который принимает по ссылке данные типа uchar data (массив байт), в данном случае испольузет адрес из 'frame', длиной '.fmt.pix.sizeimage+qstrlen(header)' (размер изображения + текстового заголовка) и форматом 'PPM' (если PPM=0, пытается сам разобрать формат по заголовку внутри)
// Объявляем указатель на беззнаковый символ (байт),  который получает адрес области памяти, выделяемой функцией malloc, получающей на входе размер изображения '.fmt.pix.sizeimage' + длину текстовой строки, хранимой в массиве 'header' ( = P6\n640 480 255\n)
        if(videovidget->img.loadFromData(cframe,wcropsize*hcropsize*pixsize+qstrlen(header),"PPM")){
            if(videovidget->isVisible()){                                               // Если предок (виджет видеоокна) имеет сатус "Видимый (isVisible),
                videovidget->update();                                                  // Вызываем у предка (видеовиджета) метод 'update', который перерисовывает изображение
            }
            if (saveframe)                                                              // Если есть требование сохранить снимок,
            {
                cs=videovidget->img;
                takeshot = false;                                                       // Сбросим требование сохранения снимка
            }
        }
        if(cframe)                                                                      // Если указатель 'frame' true, освобождаем память выделенную под 'frame', но сам указатель при этом не меняется, то есть указывает на тот же блок памяти, а не на NULL
            free(cframe);
        if (!takeshot)
            msleep(250);                                                                // Ограничиваем скорость захвата 4к/сек (4 мсек/кадр) ДОБАВЛЕНО 070120!
        }catch(...){}                                                                   // Ловим все возникающие исключения
        xioctl(fd, VIDIOC_QBUF, &buf);                                                  // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_QBUF - ставим буферы в очередь), 3:ссылка на буфер структуры запроса 'buf'
        di++;                                                                           // Увеличивем итератор  'di'
   }
    try{                                                                                // Пробуем...
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                                 // Переменной 'type' присваиваем (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    xioctl(fd, VIDIOC_STREAMOFF, &type);                                                // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_STREAMOFF - выключаем камеру), 3:ссылка переменную, хранящую тип буфера ( V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    for (unsigned int i = 0; i < n_buffers; ++i)                                        // Пока итератор меньше индекса буфера,  увеличиваем итератор
           v4l2_munmap(buffers[i].start, buffers[i].length);                            // В цикле вызываем функцию 'v4l2_munmap', которая получает на входе адрес начала текущего буфера в массиве и его длину

        v4l2_close(fd);                                                                 // Вызываем функцию 'v4l2_close', которой передаём дескриптор файла
    }catch(...){}                                                                       // Перехватываем любые исключения, никак их не обрабатывая

    if (dst_buf)
        free(dst_buf);                                                                  // Освобождаем память, выделенную под буфер приёма
    if (buffers)
        free(buffers);
    if (v4lconvert_data)
        free(v4lconvert_data);

    fd = -1;                                                                            // Возвращаем дескриптор в исходное состояние (-1)
    ctrenum=false;                                                                      // После закрытия устройства, считаем контролы не готовыми для формирования интерфейса
}


CaptureThread::~CaptureThread()                                                         // Деструктор класса (при закрытии приложения)
{
    qDebug() << "Capture destroyed!";
    if (fd == -1)
        return;
    try{                                                                                // пробуем выполнить действия, аналогичные обычному выключению камеры, но не вычищаем все буфера в цикле (закомментировано)
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                                 // Переменной 'type' присваиваем (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    xioctl(fd, VIDIOC_STREAMOFF, &type);                                                // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_STREAMOFF - выключаем камеру), 3:ссылка переменную, хранящую тип буфера ( V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    /*for (int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);*/

        v4l2_close(fd);                                                                 // Вызываем функцию 'v4l2_close', которой передаём дескриптор файла
    }catch(...){}                                                                       // Перехватываем любые исключения, никак их не обрабатывая
    ctrenum=false;                                                                      // После закрытия устройства, считаем контролы не готовыми для формирования интерфейса
    fd = -1;                                                                            // Возвращаем дескриптор в исходное состояние (-1)
}

void CaptureThread::stopUlan()                                                          // Метод 'stopUlan()' ,  используется для остановки воспроизведения или захвата изображения.
{                                                                                       // Выполняет теже операции, что и деструктор, но ко всему присваивает переменной 'capture' значение false
    capture=false;
    try{                                                                                // пробуем выполнить действия, аналогичные обычному выключению камеры, но не вычищаем все буфера в цикле (закомментировано)
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                                                 // Переменной 'type' присваиваем (V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    xioctl(fd, VIDIOC_STREAMOFF, &type);                                                // Осуществляем вызов xioctl, в котором передаём 1:Дескриптор файла, 2:типа запроса (VIDIOC_STREAMOFF - выключаем камеру), 3:ссылка переменную, хранящую тип буфера ( V4L2_BUF_TYPE_VIDEO_CAPTURE = 1)
    /*for (int i = 0; i < n_buffers; ++i)
           v4l2_munmap(buffers[i].start, buffers[i].length);*/

        v4l2_close(fd);                                                                 // Вызываем функцию 'v4l2_close', которой передаём дескриптор файла
        ctrenum=false;                                                                  // После закрытия устройства, считаем контролы не готовыми для формирования интерфейса
        fd = -1;                                                                        // Возвращаем дескриптор в исходное состояние (-1)
    }catch(...){}                                                                       // Перехватываем любые исключения, никак их не обрабатывая
}

void CaptureThread::startUlan()                                                         // Метод 'startUlan()',  используется для включения захвата видеопотока.
{
    this->start();                                                                      // Вызывает метод 'start()' данного объекта (QThread)  (начинает выполнение потока, вызовом функции 'run()' Операционная система планирует очередь с заданным приоритетом. Если поток уже запущен, система ничего не делает
}

void CaptureThread::enumerate_menu(__u32 id, QStringList &lst)
{
    memset (&querymenu, 0, sizeof (querymenu));
    querymenu.id = id;
    lst.clear();

    for (querymenu.index = queryctrl.minimum;
         querymenu.index <= (__u32)queryctrl.maximum;
         querymenu.index++) {
        if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu)) {
            lst.append(QString(QByteArray(reinterpret_cast<const char*>(querymenu.name), 32)));
        } else {
            perror ("VIDIOC_QUERYMENU");
//            exit (EXIT_FAILURE);
        }
    }
}

void CaptureThread::enumerate_menu()
{
    camctrl.clear();                                                                    // Очищаем вектор перед наполнением информацией о контролах
    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
//    queryctrl.id = V4L2_CID_BASE;
    while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {

            memset (&control, 0, sizeof(control));
            control.id = queryctrl.id;                                                  // Укажем тип запрашиваемого контрола
            xioctl(fd, VIDIOC_G_CTRL, &control);                                        // Считаем текущие значения контролов
            queryctrl.default_value = control.value;                                    // Поля дефолтных значений заполним текущими
            camctrl.append(queryctrl);                                                  // Заполняем QVector состоянием всех контролов
//            if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
//                enumerate_menu(queryctrl.id);
        }
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    if (errno == EINVAL) {
        perror ("VIDIOC_QUERYCTRL");
//        exit (EXIT_FAILURE);
    }
    ctrenum=true;                                                                       // Перечисление завершено (для построения интерфейса)
}

void CaptureThread::setValue(__u32 id, int v)
{
    memset (&control, 0, sizeof(control));
    try {
    control.id = id;
    control.value = v;
    xioctl(fd, VIDIOC_S_CTRL, &control);
    }catch(...){}
}

void CaptureThread::setDevice(const char *name)
{
    //dev_name = name;
    strcpy(dev_name, name);
}

/*Read Params From Device*/
void CaptureThread::readParameters(QString &s)
{
//    if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &device_params))
//    {
//        printf ("\"VIDIOC_QUERYCAP\" error %d, %s\n", errno, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
    char str[192];                                                                      // Нужен контроль выхода за пределы буфера!!!
    int pos;
    if (fd == -1)
    {
        sprintf(str,"Linux USB Video Class\ncapture device not found!");
    }
    else
    {
        xioctl(fd, VIDIOC_QUERYCAP, &device_params);
        pos =  snprintf(str,48,"Driver : %s\n",device_params.driver);
        pos += snprintf(str+pos,48,"Card : %s\n",device_params.card);
        pos += snprintf(str+pos,48,"Bus_info : %s\n",device_params.bus_info);
        pos += snprintf(str+pos,48,"Version : %d.%d.%d\n",
               ((device_params.version >> 16) & 0xFF),
               ((device_params.version >> 8) & 0xFF),
               (device_params.version & 0xFF));
    }
    s = QString(QByteArray(reinterpret_cast<const char*>(str)));
}

void CaptureThread::readResolutions(QStringList &resolutions)
{
    resolutions.clear();
    memset (&video_fmt, 0, sizeof(video_fmt));
    video_fmt.type = V4L2_FRMSIZE_TYPE_DISCRETE;
//    video_fmt.index = 0;
//    if (-1 == ioctl(fd, VIDIOC_ENUM_FMT, &video_fmt))
//    {
//        printf ("\"VIDIOC_ENUM_FMT\" error %d, %s\n", errno, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
    xioctl(fd, VIDIOC_ENUM_FMT, &video_fmt);
    memset (&frame_fmts, 0, sizeof(frame_fmts));
    frame_fmts.pixel_format = video_fmt.pixelformat;
    frame_fmts.type = V4L2_FRMSIZE_TYPE_DISCRETE;
//    frame_fmts.index = 1;
    frame_fmts.index = 0;
    while (0 == ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_fmts))
    {
        frame_fmts.index++;
        resolutions.append(QString::number(frame_fmts.discrete.width) + "x" + QString::number(frame_fmts.discrete.height));
    }

    if (errno == EINVAL) {
        perror ("VIDIOC_ENUM_FRAMESIZES");
        return ;
    }
        printf ("\"VIDIOC_ENUM_FRAMESIZES\" error %d, %s\n", errno, strerror(errno));
//        exit(EXIT_FAILURE);
}

void CaptureThread::setResolution(int index)
{
    memset (&video_fmt, 0, sizeof(video_fmt));
    memset (&frame_fmts, 0, sizeof(frame_fmts));
    video_fmt.type = V4L2_FRMSIZE_TYPE_DISCRETE;
    xioctl(fd, VIDIOC_ENUM_FMT, &video_fmt);
    frame_fmts.type = V4L2_FRMSIZE_TYPE_DISCRETE;
//    frame_fmts.index = ++index;                                                       //  Используем значение индекса, увеличенное на 1 для соответствия QListBoxSR
    frame_fmts.index = index++;
    frame_fmts.pixel_format = video_fmt.pixelformat;
    if (0 == ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_fmts))
    {
        snprintf(xrstr,5,"%d",frame_fmts.discrete.width);
        snprintf(yrstr,5,"%d",frame_fmts.discrete.height);
//        qDebug() << frame_fmts.discrete.height;
    }


    if (errno == EINVAL) {
        perror ("VIDIOC_ENUM_FRAMESIZES");
//        qDebug() << "succsess story";
        return ;
    }
//        printf ("\"VIDIOC_ENUM_FRAMESIZES\" error %d, %s\n", errno, strerror(errno));
//        qDebug() << "error 11";
//        exit(EXIT_FAILURE);
}
