#include "scriptstatuswidget.h"
#include "ui_scriptstatuswidget.h"

ScriptStatusWidget::ScriptStatusWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ScriptStatusWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

ScriptStatusWidget::~ScriptStatusWidget()
{
    delete ui;
}

void ScriptStatusWidget::setName(const QString& name)
{
    ui->filename->setText(name);
}

void ScriptStatusWidget::setElapsed(double s)
{
    ui->elapsed->setValue(s);
}
