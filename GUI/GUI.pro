#-------------------------------------------------
#
# Project created by QtCreator 2013-03-03T13:24:58
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    threads.cpp

HEADERS  += mainwindow.h \
    threads.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../GibbsLDA/release/ -lGibbsLDA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../GibbsLDA/debug/ -lGibbsLDA
else:unix: LIBS += -L$$OUT_PWD/../GibbsLDA/ -lGibbsLDA

INCLUDEPATH += $$PWD/../GibbsLDA
DEPENDPATH += $$PWD/../GibbsLDA

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/release/libGibbsLDA.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/debug/libGibbsLDA.a
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/libGibbsLDA.a

LIBS += -lqwt -lgsl -lgslcblas

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Munkres/release/ -lMunkres
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Munkres/debug/ -lMunkres
else:unix: LIBS += -L$$OUT_PWD/../Munkres/ -lMunkres

INCLUDEPATH += $$PWD/../Munkres
DEPENDPATH += $$PWD/../Munkres

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/release/libMunkres.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/debug/libMunkres.a
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Munkres/libMunkres.a
