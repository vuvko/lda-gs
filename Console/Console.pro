TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../GibbsLDA/release/ -lGibbsLDA
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../GibbsLDA/debug/ -lGibbsLDA
else:unix:!macx: LIBS += -L$$OUT_PWD/../GibbsLDA/ -lGibbsLDA

INCLUDEPATH += $$PWD/../GibbsLDA
DEPENDPATH += $$PWD/../GibbsLDA

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/release/libGibbsLDA.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/debug/libGibbsLDA.a
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../GibbsLDA/libGibbsLDA.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Munkres/release/ -lMunkres
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Munkres/debug/ -lMunkres
else:unix:!macx: LIBS += -L$$OUT_PWD/../Munkres/ -lMunkres

INCLUDEPATH += $$PWD/../Munkres
DEPENDPATH += $$PWD/../Munkres

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/release/libMunkres.a
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Munkres/debug/libMunkres.a
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../Munkres/libMunkres.a

LIBS += -lgsl -lgslcblas
