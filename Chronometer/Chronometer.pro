#-------------------------------------------------
#
# Project created by QtCreator 2017-04-16T22:26:20
#
#-------------------------------------------------

QT       += core gui multimedia serialport
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Chronometer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../include
INCLUDEPATH += include
SOURCES +=  src/main.cpp\
            src/MainWindow.cpp \
            src/ChronometerController.cpp \
    src/Competition.cpp \
    src/Commons.cpp \
    src/AtTinySerial.cpp

HEADERS  += include/MainWindow.h \
            include/ChronometerController.h \
            include/Commons.h \
            include/Competition.h \
    include/Athlete.h \
    include/AtTinySerial.h

FORMS    += MainWindow.ui

copydata.commands = $(COPY_DIR) $$PWD/resources $$OUT_PWD
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
