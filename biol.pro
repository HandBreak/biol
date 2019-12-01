    #-------------------------------------------------
#
# Project created by QtCreator 2019-02-19T17:56:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = biol
TEMPLATE = app

include($${_PRO_FILE_PWD_}/qjson.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    arduino.cpp \
    task.cpp \
    experiments.cpp \
    capturethread.cpp \
    videowidget.cpp \
    networksettings.cpp \
    calibratorwidget.cpp \
    tabletwidget.cpp \
    cloud.cpp \
    termostat.cpp \
    actuatorinterface.cpp \
    taskexecutor.cpp \
    information.cpp \
    qdynamicwidget.cpp

HEADERS += \
        mainwindow.h \
    arduino.h \
    task.h \
    experiments.h \
    capturethread.h \
    videowidget.h \
    networksettings.h \
    calibratorwidget.h \
    tabletwidget.h \
    cloud.h \
    termostat.h \
    actuatorinterface.h \
    taskexecutor.h \
    actuatorconstants.h \
    information.h \
    qdynamicwidget.h

FORMS += \
        mainwindow.ui \
    experiments.ui \
    networksettings.ui \
    calibratorwidget.ui

unix:!macx: LIBS += -L$$PWD/../../../../opt/tslib/lib/ -lts
LIBS += -L$$PWD/../../../../opt/v4l-utils/lib/ -lv4l2 -lv4lconvert
LIBS += -L$$PWD/../../../../opt/curl-7.55.1/lib/ -lcurl
LIBS += -L$$PWD/../../../../opt/openssl-1.0.2d/lib/ -lssl -lcrypto

INCLUDEPATH += $$PWD/../../../../opt/tslib/include
INCLUDEPATH += /opt/v4l-utils/include
INCLUDEPATH += /opt/curl-7.55.1/include
INCLUDEPATH += $$PWD/../../../../opt/openssl-1.0.2d/include/openssl

DEPENDPATH += $$PWD/../../../../opt/tslib/include
DEPENDPATH += $$PWD/../../../../opt/openssl-1.0.2d/include/openssl

# Manually added for remote execution ! (RSN)
target.path = /opt
INSTALLS += target
export (INSTALLS)

RESOURCES += \
    images.qrc
