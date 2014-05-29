#include "jsonfetcher.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

#include <iostream>

JsonFetcher::JsonFetcher(QObject *parent)
    : QObject(parent)
    , m_manager(0)
    , m_reply(0)
{
}

JsonFetcher::~JsonFetcher() {
    if (m_reply) {
       m_reply->abort();
    }
    delete m_reply;
    delete m_manager;
}

void JsonFetcher::fetchUrl(const QString& urlString)
{
    QUrl url(urlString);
    if (!url.isValid()) {
        std::cerr << "Requested url is invalid: " << urlString.toUtf8().data() << std::endl;
    }

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &JsonFetcher::gotReply);
    m_reply = m_manager->get(QNetworkRequest(url));

}

void JsonFetcher::gotReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {

        QJsonParseError err;
        QJsonDocument json = QJsonDocument::fromJson(reply->readAll(), &err);
        if (!json.isObject()) {
            std::cerr << "Failed to parse json: " << err.errorString().toUtf8().data() << std::endl;
            return;
        }

        QJsonObject obj = json.object();


        emit gotJson(obj);

    } else {
        std::cerr << "Failed to fetch url: " << reply->errorString().toUtf8().data() << std::endl;
    }

    //reply->deleteLater();
    m_reply = 0;
    //m_manager->deleteLater();
    m_manager = 0;
}
