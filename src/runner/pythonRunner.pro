#-------------------------------------------------
#
# Project created by QtCreator 2013-09-01T15:55:01
#
#-------------------------------------------------

TARGET = pythonRunner
TEMPLATE = lib

CONFIG += dll
CONFIG += warn_on

DEFINES += PYTHONRUNNER_LIBRARY

SOURCES += pythonrunner.cpp \
    gilock.cpp \
    error.cpp \
    pythonpluginwrapper.cpp \
    proxypluginwrappers.cpp

HEADERS += pythonrunner.h \
    gilock.h \
    error.h \
    uibasewrappers.h \
    pythonpluginwrapper.h \
    proxypluginwrappers.h



CONFIG(debug, debug|release) {
  LIBS += -L$$OUT_PWD/../uibase/debug
} else {
  LIBS += -L$$OUT_PWD/../uibase/release
	QMAKE_CXXFLAGS += /Zi
	QMAKE_LFLAGS += /DEBUG
}


INCLUDEPATH += "$(BOOSTPATH)" "$$(PYTHONPATH)/include" "$$(PYTHONPATH)/Lib/site-packages/PyQt4/include"
LIBS += -L"$$(PYTHONPATH)/libs" -L"$(BOOSTPATH)/stage/lib"
LIBS += -lpython27

INCLUDEPATH += ../uibase
LIBS += -luibase
