#-------------------------------------------------
#
# Project created by QtCreator 2013-04-05T18:26:04
#
#-------------------------------------------------


TARGET = proxyPython
TEMPLATE = lib

contains(QT_VERSION, "^5.*") {
  QT += widgets
}

CONFIG += plugins
CONFIG += dll

CONFIG(release, debug|release) {
  QMAKE_CXXFLAGS += /Zi
  QMAKE_LFLAGS += /DEBUG
}

DEFINES += PROXYPYTHON_LIBRARY

# suppress a few warnings caused by boost vs vc++ paranoia
DEFINES += _SCL_SECURE_NO_WARNINGS

SOURCES += proxypython.cpp
HEADERS += proxypython.h \
    resource.h

OTHER_FILES += \
    proxypython.json \
    embedrunner.rc

RC_FILE += \
    embedrunner.rc

include(../plugin_template.pri)

INCLUDEPATH += "../../pythonRunner" "$(BOOSTPATH)"

WINPWD = $$PWD
WINPWD ~= s,/,$$QMAKE_DIR_SEP,g

//QMAKE_POST_LINK += SET VS90COMNTOOLS=%VS100COMNTOOLS% $$escape_expand(\\n)


//QMAKE_POST_LINK += copy $$(PYTHONPATH)\\lib\\site-packages\\sip.pyd $$quote($$DSTDIR)\\plugins\\data\\ $$escape_expand(\\n)
//QMAKE_POST_LINK += copy $$(PYTHONPATH)\\lib\\site-packages\\PyQt4\\QtCore.pyd $$quote($$DSTDIR)\\plugins\\data\\PyQt4\\ $$escape_expand(\\n)
//QMAKE_POST_LINK += copy $$(PYTHONPATH)\\lib\\site-packages\\PyQt4\\QtGui.pyd $$quote($$DSTDIR)\\plugins\\data\\PyQt4\\ $$escape_expand(\\n)
