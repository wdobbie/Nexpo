#ifndef FINDLINEEDIT_H
#define FINDLINEEDIT_H

#include <QLineEdit>

class FindLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FindLineEdit(QWidget *parent = 0);

signals:
    void escapePressed();
    void hidden();
    void shown();

public slots:

protected:
    void keyPressEvent(QKeyEvent*);
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);
    void focusInEvent(QFocusEvent *);
};

#endif // FINDLINEEDIT_H
