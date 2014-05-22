#ifndef OUTPUTREDIRECTOR_H
#define OUTPUTREDIRECTOR_H

#include <QObject>

class QThread;

class OutputRedirector : public QObject
{
    Q_OBJECT
public:
    explicit OutputRedirector(int fd);

signals:
    void output(const QString&);

public slots:

private:
    QThread* mThread;
};

#endif // OUTPUTREDIRECTOR_H
