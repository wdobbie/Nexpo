#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

QString appRootPath();
void fatalError(const QString& msg);

class MainWindow;
class QEvent;

class Application : public QApplication
{
    Q_OBJECT
public:
    explicit Application(int&, char**);
    void setMainWindow(MainWindow*);

signals:

public slots:

protected:
    virtual bool event(QEvent *);

private:
    MainWindow* m_mainWindow;
};

#endif // APPLICATION_H
