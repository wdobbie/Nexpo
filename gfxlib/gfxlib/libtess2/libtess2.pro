TEMPLATE = lib
CONFIG += staticlib
TARGET = tess2
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += $$_PRO_FILE_PWD_/Source/*.c
HEADERS += $$_PRO_FILE_PWD_/Source/*.h \
           $$_PRO_FILE_PWD_/Include/*.h


INCLUDEPATH += $$_PRO_FILE_PWD_/Include

CONFIG(release, debug|release)  {
    windows:DESTDIR = $$_PRO_FILE_PWD_/build/windows/release
    mac:DESTDIR = $$_PRO_FILE_PWD_/build/mac/release
    linux:DESTDIR = $$_PRO_FILE_PWD_/build/linux/release
} else {
    windows:DESTDIR = $$_PRO_FILE_PWD_/build/windows/debug
    mac:DESTDIR = $$_PRO_FILE_PWD_/build/mac/debug
    linux:DESTDIR = $$_PRO_FILE_PWD_/build/linux/debug
}

windows {
    # I get an error from cl about the .pdb file, including this flag fixes it
    QMAKE_CFLAGS += -FS
}
