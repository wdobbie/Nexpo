#include "application.h"
#include "mainwindow.h"
#include <QDir>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QSettings>

QString appRootPath()
{
    QDir rootDir = QDir(QCoreApplication::applicationDirPath());

#ifdef Q_OS_MAC
    // Back out of the OS X .app bundle if we're inside it
    if (rootDir.dirName() == "MacOS") {
        rootDir.cdUp();
        if (rootDir.dirName() == "Contents") {
            rootDir.cdUp();
            if (rootDir.dirName().endsWith(".app")) {
                rootDir.cdUp();
            }
        }
    }
#else
    // cd to parent if we're in "bin"
    if (rootDir.dirName() == "bin") {
        rootDir.cdUp();
    }
#endif

    return rootDir.absolutePath();
}

QString nexpoPath() {
    QSettings settings;
    return settings.value("nexpoPath", appRootPath()).toString();
}

void fatalError(const QString& msg)
{
    QMessageBox box;
    box.setText(qApp->applicationDisplayName() + " has experienced an error and will now quit.");
    box.setInformativeText(msg);
    box.setWindowTitle("Error");
    box.setIcon(QMessageBox::Critical);
    box.exec();
    qFatal("FATAL: %s", msg.toUtf8().constData());
}


Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_mainWindow(0)
{
}

void Application::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

bool Application::event(QEvent* e)
{
    if (e->type() == QEvent::FileOpen) {
        QFileOpenEvent* fe = static_cast<QFileOpenEvent*>(e);
        if (fe && m_mainWindow) {
            m_mainWindow->loadFile(fe->file());
        }
    }
    return QApplication::event(e);
}

