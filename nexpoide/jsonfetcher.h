#ifndef JSONFETCHER_H
#define JSONFETCHER_H
#include <QString>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class JsonFetcher : public QObject
{
    Q_OBJECT
public:
    explicit JsonFetcher(QObject *parent = 0);
    ~JsonFetcher();
    void fetchUrl(const QString&);

signals:
    void gotJson(const QJsonObject&);

public slots:
    void gotReply(QNetworkReply*);

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply;
};

#endif // JSONFETCHER_H
