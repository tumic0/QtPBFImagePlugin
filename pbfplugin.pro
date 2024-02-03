TARGET  = pbf
TEMPLATE = lib
CONFIG += plugin
QT += gui
VERSION = 3.1

PROTOS = protobuf/vector_tile.proto
include(protobuf/vector_tile.pri)

INCLUDEPATH += ./protobuf
HEADERS += src/pbfhandler.h \
    src/pbfplugin.h \
    src/gzip.h \
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
    src/pbfhandler.cpp \
    src/gzip.cpp \
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

unix:!macx:!android {
    LIBS += -lprotobuf-lite \
        -lz

    target.path += $$[QT_INSTALL_PLUGINS]/imageformats
    INSTALLS += target
}
win32 {
    INCLUDEPATH += $$PROTOBUF/include \
        $$ZLIB/include
    LIBS += $$PROTOBUF/lib/libprotobuf-lite.lib \
        $$ZLIB/lib/zlibstatic.lib

    QMAKE_TARGET_PRODUCT = QtPBFImagePlugin
    QMAKE_TARGET_DESCRIPTION = Qt $$QT_VERSION MVT/PBF image plugin
    QMAKE_TARGET_COPYRIGHT = Copyright (c) 2018-2024 Martin Tuma
}
macx {
    INCLUDEPATH += $$PROTOBUF/include
    LIBS += $$PROTOBUF/lib/libprotobuf-lite.a \
        -lz
}
android {
    INCLUDEPATH += $$PROTOBUF/include
    LIBS += $$PROTOBUF/$$ANDROID_TARGET_ARCH/libprotobuf-lite.a \
        -lz

    top_builddir=$$shadowed($$PWD)
    DESTDIR = $$top_builddir/plugins
    TARGET = $$qt5LibraryTarget(libpbf, "plugins/imageformats/")
}
