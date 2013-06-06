#-------------------------------------------------
#
# Project created by QtCreator 2013-04-05T18:26:04
#
#-------------------------------------------------


TARGET = proxyPython
TEMPLATE = lib

CONFIG += plugins
CONFIG += dll
CONFIG += warn_on

DEFINES += PROXYPYTHON_LIBRARY

INCLUDEPATH += "$(BOOSTPATH)" "$$(PYTHONPATH)/include"
INCLUDEPATH += "$$(SIPPATH)/siplib"

SOURCES += proxypython.cpp \
    proxypluginwrappers.cpp \
    error.cpp \
    gilock.cpp

HEADERS += proxypython.h \
    proxypluginwrappers.h \
    uibasewrappers.h \
    error.h \
    gilock.h

LIBS += -L"$$(PYTHONPATH)/libs" -L"$(BOOSTPATH)/stage/lib" -lpython27

include(../plugin_template.pri)

OTHER_FILES += \
		INexusBridge.sip \
		interfaces.sip \
    setup.py \
    setup.cfg

WINPWD = $$PWD
WINPWD ~= s,/,$$QMAKE_DIR_SEP,g


SIPPATH=""

QMAKE_POST_LINK += SET VS90COMNTOOLS=%VS100COMNTOOLS% $$escape_expand(\\n)
