#ifndef ADDCONTROLFORM_H
#define ADDCONTROLFORM_H

#include <QDialog>

namespace Ui {
class AddControlForm;
}

class AddControlForm : public QDialog
{
    Q_OBJECT

public:
    explicit AddControlForm(QWidget *parent = 0);
    ~AddControlForm();

private:
    Ui::AddControlForm *ui;
};

#endif // ADDCONTROLFORM_H
