#ifndef METHODSETUP_H
#define METHODSETUP_H

#include <QStackedWidget>
#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QTimer>
#include <tabletwidget.h>
#include <task.h>

namespace Ui {
class MethodSetup;
}

class MethodSetup : public QStackedWidget
{
    Q_OBJECT

public:
    explicit MethodSetup(QWidget *parent = 0, Task *t = 0);
    ~MethodSetup();
    QMessageBox qmsg;                 // Диалог ожидания подогрева. Завершается автоматически по сигналу от термостата
    void showMethodCtrl();
    void showHoleSelector();
    void updateCtrlState();

private:
    Ui::MethodSetup *ui;
    TabletWidget *tabletWidget;
    Task *task;
    short x,y;

private slots:
    void changedCtrl();
    void onNextClicked();
    void onRightClicked();
    void onLeftClicked();
    void onBackClicked();
    void onPauseClicked();
    void onContinueClicked();
    void changedVisualCtrl();
    void onCloseVisualCtl();
    void onHoleToggled(QStringList);
    void curHoleNumber(QString);
    void restColor();
//    void onSelectHoleCtlToggled(bool);

signals:
    void letsStart(Task *task);
    void calibrate(Task *task);
    void setTermostat(short);
    void bottomLimit(int);
    void videoControl(bool);
    void toMainReturn();
};

#endif // METHODSETUP_H
