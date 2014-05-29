#ifndef UPDATEINFOWIDGET_H
#define UPDATEINFOWIDGET_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class UpdateInfoWidget;
}

class UpdateInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateInfoWidget(QWidget *parent = 0);
    ~UpdateInfoWidget();

public slots:
    void setInfo(const QJsonObject&);

private:
    Ui::UpdateInfoWidget *ui;
};

#endif // UPDATEINFOWIDGET_H
