TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /home/lezh1k/avr8-gnu-toolchain-linux_x86_64/avr/include
INCLUDEPATH += include

DEFINES += __AVR_ATtiny2313__

DISTFILES += \
    Makefile

SOURCES += \
    src/main.c

HEADERS += \
    include/commands_consts.h

