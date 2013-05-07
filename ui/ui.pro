QT     += core gui widgets dbus
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x

TARGET = procmon
TEMPLATE = app

SOURCES += main.cpp ui.cpp uidbus.cpp lkm_loaderiface.cpp
HEADERS  += ui.h uidbus.h lkm_loaderiface.h
FORMS    += ui.ui

QMAKE_POST_LINK += echo $$PWD/procmon $$PWD/../procmon
