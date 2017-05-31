#-------------------------------------------------
#
# Project created by QtCreator 2017-05-09T09:15:02
#
#-------------------------------------------------

QT       += core gui serialport

CONFIG   += extserialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = untitled
TEMPLATE = app
RC_ICONS = psp.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    mainwindow.cpp \
    qcustomplot.cpp \
    implement.cpp


HEADERS  += mainwindow.h \
    ljackuw.h \
    qcustomplot.h \



FORMS    += mainwindow.ui

DISTFILES += \
    ljackuw.lib \
    ljackuw.dll \
    ljackuwx.ocx \
    test.csv

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../drivers/ -lljackuw
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../drivers/ -lljackuw

INCLUDEPATH += $$PWD/../drivers
DEPENDPATH += $$PWD/../drivers
