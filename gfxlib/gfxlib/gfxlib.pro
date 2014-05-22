TEMPLATE = lib
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

INCLUDEPATH += $$_PRO_FILE_PWD_/glfw/include
INCLUDEPATH += $$_PRO_FILE_PWD_/libtess2/include
INCLUDEPATH += $$_PRO_FILE_PWD_/glm

DESTDIR = $$_PRO_FILE_PWD_/../Nexpo

mac {
    LIBS += $$_PRO_FILE_PWD_/libtess2/build/mac/release/libtess2.a
    LIBS += -L$$_PRO_FILE_PWD_/glfw-build/src -lglfw3
    LIBS += -framework Cocoa -framework OpenGL -framework CoreVideo -framework IOKit
    QMAKE_CXXFLAGS += -fvisibility=hidden
    CONFIG(release, debug|release) {

    } else {

    }
}

win32 {
    CONFIG(release, debug|release) {
        LIBS += $$_PRO_FILE_PWD_/libtess2/build/windows/release/tess2.lib
        LIBS += $$_PRO_FILE_PWD_/glfw/src/Release/glfw3.lib
    } else {
        LIBS += $$_PRO_FILE_PWD_/libtess2/build/windows/debug/tess2.lib
        LIBS += $$_PRO_FILE_PWD_/glfw/src/Debug/glfw3.lib
    }
    LIBS += -lopengl32 -luser32 -lgdi32
    DEFINES += _CRT_SECURE_NO_WARNINGS

    SOURCES +=  gl_2_1.c

}


SOURCES += shader.cpp \
    canvas.cpp \
    path.cpp \
    font.cpp

HEADERS += \
    shader.h \
    common.h \
    canvas.h \
    path.h \
    font.h
