#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QTime>

#include <task.h>
#include <tabletwidget.h>

namespace Ui {
class Experiments;
}

class Experiments : public QStackedWidget
{
    Q_OBJECT

public:
    explicit Experiments(QStackedWidget *parent = 0, Task *t = 0);
    ~Experiments();
    QMessageBox qmsg;                 // Диалог ожидания подогрева. Завершается автоматически по сигналу от термостата

signals:
    void toMainReturn();
    void letsStart(Task *task);
    void videoControl(bool);          // Для вызова видеовджета
    void calibrate(Task *task);       // Вызов калибровщика
    void bottomLimit(int);            // Установить нижнюю границу оси Z для выбранного планшета
    void setTermostat(short);

private:
    Ui::Experiments *ui;
    void selHoles12();
    void selHoles24();
    void selHoles48();
    void selHoles96();
    void setValues();
    TabletWidget *tabletWidget;
    QVBoxLayout *layout;
    Task *task;
    short x,y;

private slots:
    void onSelectHoleCtlToggled(bool);
    void onVisualCtlToggled(bool);
    void onCyclesChanged();
    void onTimeChanged();
    void onTimeOut();
    void onTabletChanged();
    void onReturnClicked();
    void onStartClicked();
    void onPauseClicked();
    void onContinueClicked();
    void onRightClicked();
    void onLeftClicked();
    void onTemperatureChanged();
    void waitEndOfExperiment(bool);
    void onCloseVisualCtl();
    void onHoleToggled(QStringList);
    void curHoleNumber(QString);
    void restColor();
};

#endif // EXPERIMENTS_H
