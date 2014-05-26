#include "addcontrolform.h"
#include "ui_addcontrolform.h"

AddControlForm::AddControlForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddControlForm)
{
    ui->setupUi(this);
}

AddControlForm::~AddControlForm()
{
    delete ui;
}
