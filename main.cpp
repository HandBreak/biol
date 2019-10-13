#include "mainwindow.h"
#include <QApplication>
#include <QScreenCursor>
#include <QTextCodec>
#include <QWSServer>
#include <QPixmap>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec *utfcodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(utfcodec);
    QTextCodec::setCodecForCStrings(utfcodec);
    QPixmap picture("://images/splash.png");

#ifdef Q_WS_QWS /* hide mouse cursor. Must be executed before first 'show()' or 'showFullScreen()' */
    QScreenCursor *cursor = new QScreenCursor;
    cursor->initSoftwareCursor();
    cursor->hide();
    QWSServer::setBackground(QBrush(picture));
#endif

    MainWindow w;
    w.showFullScreen();
    std::cout << "Running !" <<std::endl;
    return a.exec();
}
