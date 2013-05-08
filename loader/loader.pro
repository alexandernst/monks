QT              += core dbus
QT              -= gui
CONFIG          += c++11
CONFIG          -= app_bundle console
QMAKE_CXXFLAGS  += -std=c++0x

LIBS            += -lkmod

TARGET          = loader
TEMPLATE        = app

SOURCES         += main.cpp loader.cpp loaderdbus.cpp
HEADERS         += loader.h loaderdbus.h

QMAKE_POST_LINK += mkdir -p $$PWD/../build;
QMAKE_POST_LINK += cp $$PWD/loader $$PWD/../build/loader;
QMAKE_POST_LINK += cp $$PWD/com.procmon.loader.conf $$PWD/../build/com.procmon.loader.conf;
