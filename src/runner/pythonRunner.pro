#-------------------------------------------------
#
# Project created by QtCreator 2013-09-01T15:55:01
#
#-------------------------------------------------

TARGET = pythonRunner
TEMPLATE = lib

CONFIG += dll
CONFIG += warn_on
QT += widgets

DEFINES += PYTHONRUNNER_LIBRARY

# suppress a few warnings caused by boost vs vc++ paranoia
DEFINES += _SCL_SECURE_NO_WARNINGS HAVE_ROUND NOMINMAX


!include(../LocalPaths.pri) {
  message("paths to required libraries need to be set up in LocalPaths.pri")
}

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
  msvc:QMAKE_CXXFLAGS += /Zi
  msvc:QMAKE_LFLAGS += /DEBUG
}

INCLUDEPATH += "$${BOOSTPATH}" "$${PYTHONPATH}/include" "$${PYTHONPATH}/Lib/site-packages/PyQt6/include" ../uibase
LIBS += -L"$${PYTHONPATH}/libs" -L"$${BOOSTPATH}/stage/lib"
LIBS += -lpython27
LIBS += -luibase

CONFIG(debug, debug|release) {
  SRCDIR = $$OUT_PWD/debug
  DSTDIR = $$PWD/../../outputd
} else {
  SRCDIR = $$OUT_PWD/release
  DSTDIR = $$PWD/../../output
}

SRCDIR ~= s,/,$$QMAKE_DIR_SEP,g
DSTDIR ~= s,/,$$QMAKE_DIR_SEP,g

QMAKE_POST_LINK += xcopy /y /s /i $$quote($$SRCDIR\\$${TARGET}*.dll) $$quote($$DSTDIR)\\plugins\\data $$escape_expand(\\n)
QMAKE_POST_LINK += xcopy /y /I $$quote($$SRCDIR\\$${TARGET}*.pdb) $$quote($$DSTDIR)\\plugins $$escape_expand(\\n)

OTHER_FILES += \
    SConscript\
    CMakeLists.txt
