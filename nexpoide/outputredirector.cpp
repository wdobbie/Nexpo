#include "outputredirector.h"

#include <QThread>
#include <stdio.h>

#ifdef Q_OS_WIN
#include <io.h>
int pipe(int fds[2]) {
    return _pipe(fds, 1024, 0);
}
#define dup2 _dup2
#define read _read
#else
#include <unistd.h>
#endif

OutputRedirector::OutputRedirector(int fd)
{
    mThread = new QThread;
    moveToThread(mThread);

    connect(mThread, &QThread::started, [=](){
        int fds[2];
        pipe(fds);
        dup2(fds[1], fd);

        QString line;
        while(true) {
            char c;
            int len  = read(fds[0], &c, 1);
            if (len > 0) {
                line += c;
                if (c == '\n') {
                    emit output(line);
                    line.clear();
                }
            }
        }
    });
    mThread->start();
}
