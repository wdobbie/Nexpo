#include "updateinfowidget.h"
#include "ui_updateinfowidget.h"
#include <QDateTime>
#include <QUrl>

#include <iostream>

UpdateInfoWidget::UpdateInfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpdateInfoWidget)
{
    ui->setupUi(this);
}

UpdateInfoWidget::~UpdateInfoWidget()
{
    delete ui;
}

// Note: this should only ever be called once per object, since it may delete some of its widgets
void UpdateInfoWidget::setInfo(const QJsonObject& obj)
{
    QString version = obj.value("version").toString();
    QString urlString = obj.value("url").toString();
    QString message = obj.value("message").toString();
    bool validTime = obj.value("timestamp").isDouble();
    double timestamp = obj.value("timestamp").toDouble();
    qint64 msecSinceEpoch = timestamp;

    if (version.size() == 0) {
        std::cerr << "Invalid update data: missing version field" << std::endl;
        return;
    }

    QUrl url(urlString);
    QString link = urlString;
    if (url.isValid()) {
        link = QString("<a href=\"%1\">%2</a>").arg(urlString).arg(url.fileName());
    }

    if (validTime) {
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - msecSinceEpoch;
        int days = elapsed / (1000*60*60*24);
        int hours = elapsed / (1000*60*60) - (days * 24);
        QString elapsedString;
        if (days > 0) {
            elapsedString = QString("%1 day%2 ").arg(days).arg(days == 1 ? "" : "s");
        } else if (hours > 0) {
            elapsedString = QString("%1 hour%2 ").arg(hours).arg(hours == 1 ? "" : "s");
        } else {
            elapsedString = "less than an hour ";
        }
        ui->released->setText(elapsedString + "ago");
    } else {
        delete ui->released;
        delete ui->releasedLabel;
    }

    ui->version->setText(version);
    ui->download->setText(link);
    ui->download->setToolTip(urlString);
    ui->message->setText(ui->message->text() + message);

    if (urlString.size() == 0) delete ui->download;
    if (urlString.size() == 0) delete ui->downloadLabel;
    if (message.size() == 0) delete ui->message;
    if (message.size() == 0) delete ui->messageLabel;

}
