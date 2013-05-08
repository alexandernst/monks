QT              += core gui widgets dbus
CONFIG          += c++11
QMAKE_CXXFLAGS  += -std=c++0x

TARGET          = procmon
TEMPLATE        = app

SOURCES         += main.cpp loaderiface.cpp procmonui.cpp procmonuidbus.cpp
HEADERS         += loaderiface.h procmonuidbus.h procmonui.h
FORMS           += procmonui.ui

QMAKE_POST_LINK += mkdir -p $$PWD/../build;
QMAKE_POST_LINK += cp $$PWD/procmon $$PWD/../build/procmon;
QMAKE_POST_LINK += cp $$PWD/com.procmon.procmonui.conf $$PWD/../build/com.procmon.procmonui.conf;
