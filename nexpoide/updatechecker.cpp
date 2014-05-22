#include "updatechecker.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QString>
#include <iostream>

#ifdef Q_OS_MAC
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/osx.txt");
#elif defined(Q_OS_WIN)
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/windows.txt");
#elif defined(Q_OS_LINUX)
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/linux.txt");
#endif

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_manager(0)
    , m_reply(0)
{
}

UpdateChecker::~UpdateChecker() {
    if (m_reply) {
       m_reply->abort();
    }
    delete m_reply;
    delete m_manager;
}

void UpdateChecker::checkForUpdate()
{
    QUrl url(urlString);
    if (!url.isValid()) {
        std::cerr << "Cannot check for updates - url is invalid: " << urlString.toUtf8().data() << std::endl;
    }

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &UpdateChecker::gotReply);
    m_reply = m_manager->get(QNetworkRequest(url));

}

void UpdateChecker::gotReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString latestVersion = reply->readLine().trimmed();
        QString latestUrl = reply->readLine().trimmed();

        if (latestVersion.size() == 0) {
            std::cerr << "Couldn't find version string when fetching latest version info" << std::endl;
            return;
        }

        QUrl url(latestUrl);
        if (!url.isValid()) {
            std::cerr << "Got invalid url when fetching latest version info: " << latestUrl.toUtf8().data() << std::endl;
            return;
        }

        QString link = "<a href=\"" + latestUrl + "\">" + url.fileName() + "</a>";

        emit gotLatestVersion(latestVersion, link);

    } else {
        std::cerr << "Failed to fetch latest version from "
                  << urlString.toUtf8().data()
                  << " (" << reply->errorString().toUtf8().data() << ")"
                  << std::endl;
    }

    //reply->deleteLater();
    m_reply = 0;
    //m_manager->deleteLater();
    m_manager = 0;
}
