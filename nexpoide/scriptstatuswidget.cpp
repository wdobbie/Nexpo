#include "scriptstatuswidget.h"
#include "ui_scriptstatuswidget.h"

ScriptStatusWidget::ScriptStatusWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ScriptStatusWidget)
{
    ui->setupUi(this);
}

ScriptStatusWidget::~ScriptStatusWidget()
{
    delete ui;
}

void ScriptStatusWidget::setName(const QString& name)
{
    ui->filename->setText(name);
}

void ScriptStatusWidget::setElapsedSeconds(double s)
{
    ui->elapsed->setText(QString::number(s, 'f', 4));
}
