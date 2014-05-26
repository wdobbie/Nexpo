#ifndef CHECKBOXCONTROL_H
#define CHECKBOXCONTROL_H

#include "control.h"

namespace Ui {
class CheckBoxControl;
}

class CheckBoxControl : public Control
{
    Q_OBJECT

public:
    explicit CheckBoxControl(QWidget *parent = 0);
    ~CheckBoxControl();
    void setText(const QString&);

public slots:
    void setBooleanValue(const QString& name, bool);

signals:
    void toggled(bool);

private:
    Ui::CheckBoxControl *ui;
};

#endif // CHECKBOXCONTROL_H
