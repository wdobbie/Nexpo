#include "mainwindow.h"
#include "application.h"
#include <QNetworkProxyFactory>

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    a.setApplicationVersion(APP_VERSION);
    a.setOrganizationDomain("wdobbie.com");
    a.setOrganizationName("Will Dobbie");
    a.setApplicationDisplayName("Nexpo Editor");

    // Don't show menu icons on OS X
#ifdef Q_OS_MAC
    qApp->setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    // Enable use of system proxy settings for network requests
    // FIXME TODO: this is freezing on my OS X VM
    // seems to be a Qt bug? Might be blocking main thread
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    MainWindow w;
    a.setMainWindow(&w);
    w.show();

    return a.exec();
}
