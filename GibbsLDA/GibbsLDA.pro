#-------------------------------------------------
#
# Project created by QtCreator 2013-03-04T13:52:47
#
#-------------------------------------------------

QT       -= core gui

TARGET = GibbsLDA
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    dataset.cpp \
    utils.cpp \
    strtokenizer.cpp \
    model.cpp

HEADERS += \
    dataset.h \
    constants.h \
    strtokenizer.h \
    model.h \
    utils.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

LIBS += -lgsl -lgslcblas

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Munkres/release/ -lMunkres
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Munkres/debug/ -lMunkres
else:unix: LIBS += -L$$OUT_PWD/../Munkres/ -lMunkres

INCLUDEPATH += $$PWD/../Munkres
DEPENDPATH += $$PWD/../Munkres

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/release/libMunkres.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/debug/libMunkres.a
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Munkres/libMunkres.a
