QT              += core gui widgets dbus
CONFIG          += c++11
QMAKE_CXXFLAGS  += -std=c++0x

TARGET          = procmon
TEMPLATE        = app

SOURCES         += main.cpp lkm_loaderiface.cpp procmonui.cpp procmonuidbus.cpp
HEADERS         += lkm_loaderiface.h procmonuidbus.h procmonui.h
FORMS           += procmonui.ui

QMAKE_POST_LINK += cp $$PWD/procmon $$PWD/../procmon
QMAKE_POST_LINK += cp $$PWD/com.procmon.procmonui.conf $$PWD/../com.procmon.procmonui.conf
