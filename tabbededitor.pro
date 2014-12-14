isEmpty(QTC_SOURCE): QTC_SOURCE=$$(QTC_SOURCE)
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE=$$(IDE_BUILD_TREE)

include($$QTC_SOURCE/src/qtcreatorplugin.pri)

DEFINES += TABBEDEDITOR_LIBRARY
SOURCES += tabbededitorplugin.cpp
HEADERS += tabbededitorplugin.h

