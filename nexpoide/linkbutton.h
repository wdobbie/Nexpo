#ifndef LINKBUTTON_H
#define LINKBUTTON_H

#include <QToolButton>

// Styling for this class is done in MainWindow's stylesheet

class LinkButton : public QToolButton
{
    Q_OBJECT
public:
    explicit LinkButton(QWidget *parent = 0);

signals:

public slots:

};

#endif // LINKBUTTON_H
