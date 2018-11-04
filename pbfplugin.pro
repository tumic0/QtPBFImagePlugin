TARGET  = pbf
TEMPLATE = lib
CONFIG += plugin
QT += gui widgets

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
    src/textitem.h
SOURCES += src/pbfplugin.cpp \
    src/pbfhandler.cpp \
    src/gzip.cpp \
    src/pbf.cpp \
    src/style.cpp \
    src/color.cpp \
    src/text.cpp \
    src/function.cpp \
    src/textpathitem.cpp \
    src/textitem.cpp
RESOURCES += pbfplugin.qrc

unix {
    LIBS += -lprotobuf-lite \
        -lz
}
win32 {
    INCLUDEPATH += $$PROTOBUF/include \
        $$ZLIB/include
    LIBS += $$PROTOBUF/lib/libprotobuf-lite.lib \
        $$ZLIB/lib/zlibstatic.lib
}

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
