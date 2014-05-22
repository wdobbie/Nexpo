#include "aboutform.h"
#include "ui_aboutform.h"
#include "application.h"
#include <QFileInfo>

AboutForm::AboutForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutForm)
{
    ui->setupUi(this);
    setWindowTitle(qApp->applicationDisplayName());

    ui->appName->setText(qApp->applicationDisplayName());
#ifdef QT_DEBUG
    ui->version->setText(QString("%1 (debug build)").arg(qApp->applicationVersion()));
#else
    ui->version->setText(qApp->applicationVersion());
#endif
    ui->buildDate->setText(QString("%1, %2").arg(__DATE__).arg(__TIME__));

    ui->platform->setText(QString("Qt %1 %2")
                          .arg(QT_VERSION_STR)
                          .arg(qApp->platformName()));

    QString appPath = appRootPath();
    QString shortPath = appPath.left(40);
    if (shortPath.size() < appPath.size()) {
        shortPath = shortPath + "…";
    }

    QString apploc = QString("<a href=\"file://%1\">%2</a>")
            .arg(appPath)
            .arg(shortPath);

    ui->path->setText(apploc);
}

AboutForm::~AboutForm()
{
    delete ui;
}
