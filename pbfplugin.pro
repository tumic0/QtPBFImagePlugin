TARGET  = pbf
TEMPLATE = lib
CONFIG += plugin
QT += gui
VERSION = 5.1

HEADERS += src/pbfhandler.h \
    src/data.h \
    src/pbfplugin.h \
    src/pbf.h \
    src/style.h \
    src/color.h \
    src/text.h \
    src/tile.h \
    src/function.h \
    src/textpathitem.h \
    src/textpointitem.h \
    src/font.h \
    src/textitem.h \
    src/sprites.h
SOURCES += src/pbfplugin.cpp \
    src/data.cpp \
    src/pbfhandler.cpp \
    src/pbf.cpp \
    src/style.cpp \
    src/color.cpp \
    src/text.cpp \
    src/function.cpp \
    src/textpathitem.cpp \
    src/textpointitem.cpp \
    src/font.cpp \
    src/sprites.cpp \
    src/textitem.cpp
RESOURCES += pbfplugin.qrc

DEFINES += QT_NO_DEPRECATED_WARNINGS

unix:!android {
    target.path += $$[QT_INSTALL_PLUGINS]/imageformats
    INSTALLS += target
}
win32 {
    QMAKE_TARGET_PRODUCT = QtPBFImagePlugin
    QMAKE_TARGET_DESCRIPTION = Qt $$QT_VERSION MVT/PBF image plugin
    QMAKE_TARGET_COPYRIGHT = Copyright (c) 2018-2025 Martin Tuma
}
android {
    top_builddir=$$shadowed($$PWD)
    DESTDIR = $$top_builddir/plugins
    TARGET = $$qt5LibraryTarget(libpbf, "plugins/imageformats/")
}
