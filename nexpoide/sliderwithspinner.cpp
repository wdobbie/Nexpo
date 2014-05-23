#include "sliderwithspinner.h"
#include "ui_sliderwithspinner.h"

SliderWithSpinner::SliderWithSpinner(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SliderWithSpinner)
    , m_minValue(0)
    , m_maxValue(1)
    , m_value(1)
{
    ui->setupUi(this);
    setSliderValue();
}

SliderWithSpinner::~SliderWithSpinner()
{
    delete ui;
}

void SliderWithSpinner::setMinValue(double arg)
{
    m_minValue = arg;
    ui->spinner->setMinimum(m_minValue);
    setSliderValue();
}

void SliderWithSpinner::setMaxValue(double arg)
{
    m_maxValue = arg;
    ui->spinner->setMaximum(m_maxValue);
    setSliderValue();
}

void SliderWithSpinner::setValue(double arg)
{
    if (arg == m_value) return;
    m_value = arg;

    // Set slider
    setSliderValue();

    // Set spinner
    // Adjust spinner min/max if necessary so we can always see the true number displayed
    if (m_value > ui->spinner->maximum()) {
        ui->spinner->setMaximum(m_value);
    } else if (m_value < ui->spinner->minimum()) {
        ui->spinner->setMinimum(m_value);
    }
    ui->spinner->setValue(m_value);
}

void SliderWithSpinner::setUnits(const QString& units)
{
    ui->spinner->setSuffix(units);
}

void SliderWithSpinner::setSliderValue()
{
    // Don't want slider to change m_value when we update it
    ui->slider->blockSignals(true);

    // Set slider
    if (m_maxValue > m_minValue) {
        double percent = (m_value - m_minValue) / (m_maxValue - m_minValue);
        ui->slider->setValue((ui->slider->maximum() - ui->slider->minimum()) * percent);
    } else {
        ui->slider->setValue((ui->slider->maximum() - ui->slider->minimum()+1) / 2);
    }

    ui->slider->blockSignals(false);
}

void SliderWithSpinner::on_slider_valueChanged(int value)
{
    double percent = value / (double)(ui->slider->maximum());
    m_value = m_minValue + percent * (m_maxValue - m_minValue);

    // Set spinner
    ui->spinner->setValue(m_value);

    emit valueChanged(m_value);
}

void SliderWithSpinner::on_spinner_valueChanged(double arg)
{
    m_value = arg;
    setSliderValue();
    emit valueChanged(arg);
}
