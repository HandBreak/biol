#ifndef TASK_H
#define TASK_H
/*
 * Класс описывает параметры передаваемые в качестве задания для проведения эксперимента.
 *
 * TODO:    Предусмотреть сохранение пресетов в памяти устройства с последующим выбором
 *          предустановленного варианта.
 */
#include <QString>

class Task
{
public:
    Task();
    ~Task();

    QString methodName;              // Название метода
    QString methodCode;              // Идентификатор метода

    short tabletColumns;             // Количество лунок планшета по горизонтали (1,2,4,8,12)     Ось X
    short tabletRows;                // Количество лунок планшета по вертикали (1,2,4,8)          Ось Y

    int holeFirstXPosition;          // Позиция первой лунки по оси X (абсолютное значение)
    int holeFirstYPosition;          // Позиция первой лунки по оси Y (абсолютное значение)
    int cameraPosition;              // Позиция камеры в эксперименте
    int cameraBottomLimit;           // Минимально допустимая высота снижения камеры для данного типа планшета

    int holeXSpacing;                // Интервал между лунками по оси Х
    int holeYSpacing;                // Интервал между лунками по оси Y
    int shiftCorrection;             // Коррекция смещения по Y (в мм / 1000  между крайними лунками)   ПОКА НЕ ИСПОЛЬУЕТСЯ !!!

    bool horizontMovement;           // Порядок прохода лунок (true - построчно, false - по колонкам)
    bool doingShake;                 // Производить встряхивание перед съемкой
    short shotPerCycle;              // Количество снимков каждой лунки в одном цикле
    short shotInterval;              // Интервал между двумя снимками лунки в одном цикле (в мсек), он же ожидание после встряхивания
    short numberCycles;              // Количество циклов съемки (0, если ограничено только временем)
    int experimentTime;              // Продолжительность эксперимента (мсек) (0, если ограничено только числом циклов);
    short temperatureSet;            // Установить температуру (если 0, подогрев отключен)

    short startHole[2];              // Номер стартовой лунки [x,y] Отсчет с нуля [(0 - 11),(0 - 7)] эквивалент [01-12, A-H]
    short endHole[2];                // Номер конечной лунки [x,y] Отсчтет с нуля [(0 - 11),(0 - 7)] эквивалент [01-12, A-H]
    bool excludedHole[12][8];        // Двумерный массив исключенных из опыта ячеек

    bool continueFlag;               // Флаг, признак продолжения опыта
    bool pauseEnabled;               // Флаг, признак приостановки опыта
    bool openTray;                   // Флаг требования выдвинуть лоток
    bool sendToCloud;                // Флаг требования отправки в облако
};
#endif // TASK_H
