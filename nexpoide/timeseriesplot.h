#ifndef TIMESERIESPLOT_H
#define TIMESERIESPLOT_H

#include "control.h"
#include <qcustomplot.h>

#include <QVector>

namespace Ui {
class TimeSeriesPlot;
}

class TimeSeriesPlot : public Control
{
    Q_OBJECT

public:
    explicit TimeSeriesPlot(QWidget *parent = 0);
    ~TimeSeriesPlot();
    QCustomPlot* plot();
    void addValue(double, double);
    void setTitle(const QString&);
    void setWindowSize(int);
    void setMinimumPlotInterval(double);

    ControlType type() const { return ControlType::TimeSeriesPlot; }

public slots:
    void clear();
    void replot(double);

private:
    Ui::TimeSeriesPlot *ui;
    QVector<double> m_ydata;
    QVector<double> m_xdata;
    double m_upper, m_lower;
    bool m_modified;
    int m_nextSlot;
    qint64 m_plotInterval;      // in milliseconds
    qint64 m_lastPlotTime;      // milliseconds since epoch
};

#endif // TIMESERIESPLOT_H
