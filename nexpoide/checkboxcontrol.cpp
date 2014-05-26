#include "checkboxcontrol.h"
#include "ui_checkboxcontrol.h"

CheckBoxControl::CheckBoxControl(QWidget *parent) :
    Control(parent),
    ui(new Ui::CheckBoxControl)
{
    ui->setupUi(this);
    connect(ui->checkBox, &QCheckBox::toggled, this, &CheckBoxControl::toggled);
}

CheckBoxControl::~CheckBoxControl()
{
    delete ui;
}

void CheckBoxControl::setBooleanValue(const QString& name, bool flag)
{
    if (name == m_name) {
        ui->checkBox->setChecked(flag);
    }
}

void CheckBoxControl::setText(const QString& text)
{
    ui->checkBox->setText(text);
}
