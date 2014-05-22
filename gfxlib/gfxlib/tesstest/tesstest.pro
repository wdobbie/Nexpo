TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    path.cpp



INCLUDEPATH += $$_PRO_FILE_PWD_/libtess2/include
INCLUDEPATH += $$_PRO_FILE_PWD_/glm

LIBS += $$_PRO_FILE_PWD_/libtess2/build/mac/release/libtess2.a

HEADERS += \
    path.h
