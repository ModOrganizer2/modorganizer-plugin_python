#-------------------------------------------------
#
# Project created by QtCreator 2013-04-05T18:26:04
#
#-------------------------------------------------


TARGET = proxyPython
TEMPLATE = lib

CONFIG += plugins
CONFIG += dll

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

#QMAKE_POST_LINK += $$(PYTHONPATH)/Lib/site-packages/PyQt4/sip.exe -I "$$(PYTHONPATH)/Lib/site-packages/PyQt4/sip/PyQt4" -c . -t WS_WIN -t Qt_4_8_4 -x Py_v2 -b interfaces.sbf -g interfaces.sip $$escape_expand(\\n)
QMAKE_POST_LINK += $$(PYTHONPATH)/python.exe $$WINPWD\\setup.py build_ext --sip-opts=\"-I C:\\Python27\\Lib\\site-packages\\PyQt4\\sip\\PyQt4 -e -g -t WS_WIN -t Qt_4_8_4\" build $$escape_expand(\\n)
QMAKE_POST_LINK += copy $$WINPWD\\build\\lib.win32-2.7\\interfaces.pyd $$quote($$DSTDIR)\\plugins\\data\\ $$escape_expand(\\n)
