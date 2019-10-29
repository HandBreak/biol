#ifndef ACTUATORCONSTANTS_H
#define ACTUATORCONSTANTS_H

enum {MODE, LIGHT, COOLER, HEATER, MOTOR, DATA, TIMEOUT};
enum {X_AXIS, Y_AXIS, Z_AXIS};
enum {EXTRUDER, TABLE};
enum {STATUS = 4};

typedef unsigned short int USHORT;   // На память о возможности сокращений :)
typedef short int SHORT;             //

// -------------- Константы определяюшие параметры контроллера приводов (ActuatorInterface) ------------------------->!
/* Для настойки порта ипользовать stty -F /dev/ttyS1 с параметрами:
                                     speed 115200 baud; line = 0;
                                     min = 0; time = 10;
                                     -brkint -icrnl -imaxbel
                                     -onlcr
                                     -icanon -echo
*/
const QString com = "/dev/ttyS1";    // Коммуникационный порт связи с контроллером исполнительных устройств
const char *a;         // Добавлено для обучния работы с git
// -------------- Константы определяюшие параметры контроллера приводов (ActuatorInterface) -------------------------<!

// -------------- Константы определяюшие поведение связанное с хранением результатов эксперимента (MainWindow) ------>!
#define CLOUDURL       "https://0.biol.cloud:8080/remote.php/webdav/"
#define CLOUDPWD       "a7902a01"
#define CLOUDUSR       "a7902a01"

const bool autoclean      = true;
const bool asynchronous  = false;
const bool usetempfolder = false;
// -------------- Константы определяюшие поведение связанное с хранением результатов эксперимента (MainWindow) ------<!

// -------------- Константы определяюшие параметры встряхивания (Класс TaskExecutor) -------------------------------->!
const short shakeCycles      = 8;    // Количество циклов встряхивания
const int shakePause        = 50;    // Пауза в цикле встряхивания (мсек)
const int shakeShift      = 8000;    // Определяем максимальное смещение стола при встряхивании в мм*devider
// -------------- Константы определяюшие параметры встряхивания (Класс TaskExecutor) --------------------------------<!

// -------------- Константы определяюшие параметры управления приводами (Класс Arduino) ----------------------------->!
#define Zmin               65000;    // Нижняя граница безопасного перемещения по оси Z
const float devider       = 1000;    // Определяем делитель для перевода входных целых значений в дробные единицы  (мм)
                                     // позиционирования
const int defXpos       = 150000;    // Крайняя правая позиция по умолчанию (в мм*devider)
const int defYpos       = -75000;    // Крайняя дальняя позиция по умолчанию (в мм*devider)
const int defZpos       = 153000;    // Крайняя верхняя позиция по умолчанию (в мм*devider)

const int maxXpos       = 150000;    // Крайняя правая позиция по умолчанию (в мм*devider)
const int maxYpos       = 150000;    // Крайняя ближняя позиция по умолчанию (в мм*devider)
const int maxZpos       = 153000;    // Крайняя верхняя позиция по умолчанию (в мм*devider)

const USHORT defspeed     = 3000;    // Установим скорость по умолчанию (мм/м)
const USHORT eXspeed      = 9000;    // Скорость подачи по X в процессе эксперимента
const USHORT eYspeed      = 5000;    // Скорость подачи по Y в процессе эксперимента
const USHORT speedWhome   = 2000;    // Скорость для выполнения калибровки оси X
const USHORT speedDhome   = 2000;    // Скорость для выполнения калибровки оси Y
const USHORT speedHhome   =  300;    // Скорость для выполнения калибровки оси Z

const USHORT overWlimits   = 160;    // Сдвиг (в мм), гарантирующий выход за пределы поля ширины  (X)   (max = 156.374)
const USHORT overDlimits   = 160;    // Сдвиг (в мм), гарантирующий выход за пределы поля глубины (Y)   (max = 150.925)
const USHORT overHlimits   = 160;    // Сдвиг (в мм), гарантирующий выход за пределы поля высоты  (Z)   (max = 165.901)
// -------------- Константы определяюшие параметры управления приводами (Класс Arduino) -----------------------------<!

// -------------- Константы определяюшие параметры поведения виджета калибровки (Класс CalibratorWidget) ------------>!
const USHORT pauseWhome    = 400;    // Пауза при калибровке по оси X (сек)
const USHORT pauseDhome    = 400;    // Пауза при калибровке по оси Y (сек)
const USHORT pauseHhome    = 300;    // Пауза при калибровке по оси Z (сек)
// -------------- Константы определяюшие параметры поведения виджета калибровки (Класс CalibratorWidget) ------------<!

#endif // ACTUATORCONSTANTS_H
