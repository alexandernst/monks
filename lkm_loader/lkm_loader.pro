QT              += core dbus
QT              -= gui
CONFIG          += c++11
CONFIG          -= app_bundle console
QMAKE_CXXFLAGS  += -std=c++0x

LIBS            += -lkmod

TARGET          = lkm_loader
TEMPLATE        = app

SOURCES         += main.cpp lkm_loader.cpp lkm_loaderdbus.cpp
HEADERS         += lkm_loader.h lkm_loaderdbus.h

QMAKE_POST_LINK += cp $$PWD/lkm_loader $$PWD/../lkm_loader
QMAKE_POST_LINK += cp $$PWD/com.procmon.lkm_loader.conf $$PWD/../com.procmon.lkm_loader.conf
