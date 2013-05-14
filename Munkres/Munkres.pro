#-------------------------------------------------
#
# Project created by QtCreator 2013-05-01T12:58:20
#
#-------------------------------------------------

QT       -= core gui

TARGET = Munkres
TEMPLATE = lib
CONFIG += staticlib

DEFINES += MUNKRES_LIBRARY

SOURCES += \
    munkres.cpp \
    matrix.cpp

HEADERS +=\
    munkres.h \
    matrix.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
