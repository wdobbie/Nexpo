#include "linkbutton.h"

LinkButton::LinkButton(QWidget *parent) :
    QToolButton(parent)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setCursor(Qt::PointingHandCursor);
}
