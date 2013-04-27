QT       += core gui
CONFIG += c++11
LIBS += -lkmod

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = procmon
TEMPLATE = app
MAKEFILE = "Makefile-ui"

SOURCES += main.cpp\
        procmon.cpp

HEADERS  += procmon.h

FORMS    += procmon.ui
