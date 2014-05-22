#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H
#include <QString>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = 0);
    ~UpdateChecker();
    void checkForUpdate();

signals:
    void gotLatestVersion(const QString& versionString, const QString& link);

public slots:
    void gotReply(QNetworkReply*);

private:
    QNetworkAccessManager* m_manager;
    QNetworkReply* m_reply;
};

#endif // UPDATECHECKER_H
