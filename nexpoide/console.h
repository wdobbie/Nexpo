#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QStringList>

class QCompleter;

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Console(QWidget *parent = 0);
    ~Console();

    void setPrefix(const QString&);
    void setPrefixFont(const QFont&);
    void setPrefixColor(const QColor&);
    QString currentWord() const;
    void setCompleter(QCompleter*);

signals:
    void command(const QByteArray&);

public slots:
    void print(const QString&);
    void printError(const QString&);
    void cls();
    void insertCompletion(const QString&);

private:
    bool inCommandLine() const;
    virtual void keyPressEvent(QKeyEvent *event);
    void processCommand();
    void prepareCommandLine();
    void moveToEndOfCommandLine();
    void printWithFormat(const QString& str, const QTextCharFormat& fmt = QTextCharFormat());
    void applyFormatCode(QTextCursor&, unsigned int);

    QString m_prefix;
    bool m_commandLineReady;
    QFont m_prefixFont;
    QColor m_prefixColor;
    QColor m_errorColor;
    QStringList m_history;
    int m_historyPos;
    QCompleter* m_completer;

};

#endif // CONSOLE_H
