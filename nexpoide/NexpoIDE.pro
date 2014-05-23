#-------------------------------------------------
#
# Project created by QtCreator 2014-05-09T15:38:31
#
#-------------------------------------------------

QT       += core gui network

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "NexpoEditor"
TEMPLATE = app
VERSION = 14.5.2

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Disable debug output on release builds
#CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp\
        mainwindow.cpp \
        fileeditor.cpp \
        outputredirector.cpp \
    console.cpp \
    findlineedit.cpp \
    updatechecker.cpp \
    aboutform.cpp \
    application.cpp \
    scriptstatuswidget.cpp \
    sliderwithspinner.cpp \
    tabwidget.cpp \
    tabbar.cpp

HEADERS  += mainwindow.h \
        fileeditor.h \
        outputredirector.h \
    console.h \
    findlineedit.h \
    updatechecker.h \
    aboutform.h \
    application.h \
    scriptstatuswidget.h \
    sliderwithspinner.h \
    tabwidget.h \
    tabbar.h

FORMS    += mainwindow.ui \
    aboutform.ui \
    scriptstatuswidget.ui \
    scriptcontrols.ui \
    sliderwithspinner.ui

RESOURCES += \
    resources.qrc

DESTDIR = $$_PRO_FILE_PWD_/../gfxlib/Nexpo

mac {
    # QScintilla2
    INCLUDEPATH += $$_PRO_FILE_PWD_/QScintilla-mac/Qt4Qt5
    LIBS += $$_PRO_FILE_PWD_/QScintilla-mac/build/mac/release/libqscintilla2.a

    CONFIG(release, debug|release) {

    } else {

    }

    ICON = images/icon.icns

    OTHER_FILES += Info.plist
    QMAKE_INFO_PLIST=Info.plist
}

windows {
    CONFIG(release, debug|release) {

        LIBS += $$_PRO_FILE_PWD_/QScintilla-windows/build/windows/release/qscintilla2.lib
    } else {

        LIBS += $$_PRO_FILE_PWD_/QScintilla-windows/build/windows/debug/qscintilla2.lib
    }

    # QScintilla2
    INCLUDEPATH += $$_PRO_FILE_PWD_/QScintilla-windows/Qt4Qt5
}

