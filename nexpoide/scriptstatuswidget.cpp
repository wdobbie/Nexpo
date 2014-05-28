#include "scriptstatuswidget.h"
#include "ui_scriptstatuswidget.h"

ScriptStatusWidget::ScriptStatusWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::ScriptStatusWidget)
    , m_lastElapsedValue(-1)
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

void ScriptStatusWidget::setElapsed(double elapsed)
{
    // Only update if there is an integer increase, or if there is a decrease (eg when a new script starts)
    if (elapsed < m_lastElapsedValue || (int)elapsed > (int)m_lastElapsedValue) {
        int total = (int)elapsed;
        int min = total / 60;
        int sec = total % 60;
        ui->elapsed->setText(QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')));
        m_lastElapsedValue = elapsed;
    }
}
