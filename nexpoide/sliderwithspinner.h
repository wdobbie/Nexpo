#ifndef SLIDERWITHSPINNER_H
#define SLIDERWITHSPINNER_H

#include "control.h"

namespace Ui {
class SliderWithSpinner;
}

class SliderWithSpinner : public Control
{
    Q_OBJECT
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(double maxValue READ minValue WRITE setMaxValue)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit SliderWithSpinner(QWidget *parent = 0);
    ~SliderWithSpinner();

    double minValue() const
    {
        return m_minValue;
    }

    double value() const
    {
        return m_value;
    }

    ControlType type() const { return ControlType::Slider; }

public slots:
    void setMinValue(double arg);
    void setMaxValue(double arg);
    void setValue(double arg);
    void setUnits(const QString&);

signals:
    void valueChanged(double);

private slots:
    void on_slider_valueChanged(int value);

    void on_spinner_valueChanged(double arg1);

private:
    void setSliderValue();

    Ui::SliderWithSpinner *ui;
    double m_minValue;
    double m_maxValue;
    double m_value;
};

#endif // SLIDERWITHSPINNER_H
