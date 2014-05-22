TEMPLATE = app
CONFIG += console
CONFIG += app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp

INCLUDEPATH += $$_PRO_FILE_PWD_/luajit/src

DESTDIR = $$_PRO_FILE_PWD_/../gfxlib/Nexpo

mac {
    # For LuaJIT
    LIBS += -pagezero_size 10000 -image_base 100000000

    CONFIG(release, debug|release) {
        LIBS += $$_PRO_FILE_PWD_/luajit/mac/release/libluajit.a
    } else {
        LIBS += $$_PRO_FILE_PWD_/luajit/mac/debug/libluajit.a
    }

    ICON = icon.icns

    OTHER_FILES += Info.plist
    QMAKE_INFO_PLIST=Info.plist
}

windows {
    CONFIG(release, debug|release) {
        LIBS += $$_PRO_FILE_PWD_/luajit/windows/release/lua51.lib
    } else {
        LIBS += $$_PRO_FILE_PWD_/luajit/windows/debug/lua51.lib
    }
}
