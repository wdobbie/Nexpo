#ifndef CONTROL_H
#define CONTROL_H

#include <QWidget>

class Control : public QWidget
{
    Q_OBJECT
public:
    explicit Control(QWidget *parent = 0);

    enum ControlType {
        Slider,
        TimeSeriesPlot,
    };

    virtual ControlType type() const = 0;
    void setName(const QString& name) { m_name = name; }
signals:

public slots:

protected:
    QString m_name;
};

#endif // CONTROL_H
