#include "updatechecker.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <iostream>

#ifdef Q_OS_MAC
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/osx.json");
#elif defined(Q_OS_WIN)
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/windows.json");
#elif defined(Q_OS_LINUX)
    static const QString urlString = QStringLiteral("http://wdobbie.com/nexpo/download/linux.json");
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

        QJsonParseError err;
        QJsonDocument json = QJsonDocument::fromJson(reply->readAll(), &err);
        if (!json.isObject()) {
            std::cerr << "Failed to parse update data: " << err.errorString().toUtf8().data() << std::endl;
            return;
        }

        QJsonObject obj = json.object();
        QString version = obj.value("version").toString();
        QString url = obj.value("url").toString();
        QString message = obj.value("message").toString();

        if (version.size() == 0) {
            std::cerr << "Update check failed: missing data" << std::endl;
            return;
        }

        emit gotLatestVersion(version, url, message);

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
