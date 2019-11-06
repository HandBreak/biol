!greaterThan(QT_MAJOR_VERSION, 4): {

    INCLUDEPATH += \
        $$PWD

    SOURCES += \
        $$PWD/../qjson-backport/qjson.cpp \
        $$PWD/../qjson-backport/qjsonarray.cpp \
        $$PWD/../qjson-backport/qjsondocument.cpp \
        $$PWD/../qjson-backport/qjsonobject.cpp \
        $$PWD/../qjson-backport/qjsonparser.cpp \
        $$PWD/../qjson-backport/qjsonvalue.cpp \
        $$PWD/../qjson-backport/qjsonwriter.cpp

    HEADERS += \
        $$PWD/../qjson-backport/qjson_p.h \
        $$PWD/../qjson-backport/qjsonarray.h \
        $$PWD/../qjson-backport/qjsondocument.h \
        $$PWD/../qjson-backport/qjsonobject.h \
        $$PWD/../qjson-backport/qjsonparser_p.h \
        $$PWD/../qjson-backport/qjsonvalue.h \
        $$PWD/../qjson-backport/qjsonwriter_p.h

}

