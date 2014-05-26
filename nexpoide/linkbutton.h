#ifndef LINKBUTTON_H
#define LINKBUTTON_H

#include <QToolButton>

// This class has no additional functionality over QToolButton
// It exists purely as a target for class selectors in the
// main window's stylesheet.

class LinkButton : public QToolButton
{
    Q_OBJECT
public:
    explicit LinkButton(QWidget *parent = 0);

signals:

public slots:

};

#endif // LINKBUTTON_H
