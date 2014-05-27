#include "timeseriesplot.h"
#include "ui_timeseriesplot.h"

#include <QDateTime>
#include <cmath>

TimeSeriesPlot::TimeSeriesPlot(QWidget *parent)
    : Control(parent)
    , ui(new Ui::TimeSeriesPlot)
    , m_modified(false)
    , m_nextSlot(0)
    , m_plotInterval(0)
    , m_lastPlotTime(0)
{
    ui->setupUi(this);
    ui->plot->setBackground(QPalette().brush(QPalette::Background));
    ui->plot->addGraph();
    //ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    ui->plot->graph(0)->setLineStyle(QCPGraph::lsStepLeft);
    //ui->plot->xAxis->setTicks(false);
    //ui->plot->xAxis->setVisible(false);
    ui->plot->graph(0)->setAntialiased(false);
    ui->plot->xAxis->setLabel("Elapsed time (s)");
    ui->plot->xAxis->setRange(0, 1e-20);
    ui->plot->legend->setVisible(true);
    ui->plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft|Qt::AlignTop);
}

TimeSeriesPlot::~TimeSeriesPlot()
{
    delete ui;
}

QCustomPlot *TimeSeriesPlot::plot()
{
    return ui->plot;
}

void TimeSeriesPlot::addValue(double time, double value)
{
    if (m_nextSlot < m_ydata.size()) {
        m_ydata[m_nextSlot] = value;
        m_xdata[m_nextSlot] = time;
        m_nextSlot++;
    } else {
        m_ydata.pop_front();
        m_ydata.append(value);
        m_xdata.pop_front();
        m_xdata.append(time);
        m_nextSlot = m_ydata.count();
    }

    // Update range
    if (!m_modified) {
       m_lower = value;
       m_upper = value;
       m_modified = true;
    }

    if (value < m_lower)  m_lower = value;
    if (value > m_upper) m_upper = value;
}

void TimeSeriesPlot::replot(double elapsed)
{
    if (!m_modified) return;

    // replot if enough time elapsed
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - m_lastPlotTime > m_plotInterval) {
        // Update axes ranges
        ui->plot->xAxis->setRange(m_xdata[0], elapsed);
        double buffer = qMax(1e-20, (m_upper - m_lower) * 0.1);
        ui->plot->yAxis->setRange(m_lower - buffer, m_upper + buffer);

        // Plot
        m_lastPlotTime = now;
        ui->plot->graph(0)->setData(m_xdata, m_ydata);
        ui->plot->replot();
    }
}

void TimeSeriesPlot::clear()
{
    m_nextSlot = 0;
    m_modified = false;
}

void TimeSeriesPlot::setTitle(const QString& title)
{
    ui->plot->graph(0)->setName(title);
}

void TimeSeriesPlot::setWindowSize(int n)
{
    if (n <= 0 || n == m_ydata.count()) return;

    if (n > m_ydata.count()) {
        m_ydata.resize(n);
    } else {
        unsigned int deleteCount = n - m_ydata.count();
        m_ydata.erase(m_ydata.begin(), m_ydata.begin() + deleteCount);
        if (m_nextSlot > m_ydata.count()) m_nextSlot = m_ydata.count();
    }

    m_xdata.resize(m_ydata.size());
}

void TimeSeriesPlot::setMinimumPlotInterval(double seconds)
{
    m_plotInterval = seconds * 1000.0;
}
