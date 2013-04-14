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

INCLUDEPATH += "$(BOOSTPATH)" "C:/Program Files (x86)/Python2.7/include"

SOURCES += proxypython.cpp \
    pythontoolwrapper.cpp \
    pythonpluginwrapper.cpp

HEADERS += proxypython.h \
    pythontoolwrapper.h \
    pythonpluginwrapper.h

LIBS += -L"C:/Program Files (x86)/Python2.7/libs" -L"$(BOOSTPATH)/stage/lib" -lpython27

include(../plugin_template.pri)

OTHER_FILES += \
    iorganizer.sip \
    configure.py

QMAKE_POST_LINK += xcopy /y /s /i *.py $$quote($$DSTDIR)\\plugins $$escape_expand(\\n)
