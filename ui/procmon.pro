QT     += core gui widgets dbus
CONFIG += c++11

TARGET = procmon
TEMPLATE = app

SOURCES += main.cpp procmon.cpp procmondbus.cpp lkm_loaderiface.cpp
HEADERS  += procmon.h procmondbus.h lkm_loaderiface.h
FORMS    += procmon.ui
