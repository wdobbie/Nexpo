#ifndef FILEEDITOR_H
#define FILEEDITOR_H

#include <Qsci/qsciscintilla.h>

class QFileInfo;
class QKeyEvent;
class QsciAPIs;
class QsciLexer;
class QsciLexerLua;
class QsciLexerCPP;
class QsciLexerMatlab;
class QsciLexerJavaScript;
class QsciLexerHTML;

struct EditorPosition {
    int line, index, hscroll, vscroll;
};

class FileEditor : public QsciScintilla
{
    Q_OBJECT

public:
    explicit FileEditor(QWidget *parent = 0);
    ~FileEditor();

    bool open(const QString& path);
    QString title() const;
    void setTitle(const QString&);
    QString path() const;

    static void init(QObject* parent);
    static void addApiEntry(const QString&);
    static void prepareApi();

signals:
    void titleChanged(const QString&);
    void escapePressed();
    void statusMessageChanged(const QString& status, int timeout = 0);
    void gotFocus();
    void lostFocus();

public slots:
    bool saveFile();
    bool saveFileAs(const QString&);
    void selectLines(int first, int last);
    void highlightFindText(const QString&, bool scrollIntoView = true);
    void unhighlightFindText();
    bool find(const QString&, bool forward=true);
    bool findNextForward();
    bool findNextBackward();

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent*);

private:
    void installLexer(const QString&);
    void setFileInfo(const QFileInfo* info);

    static QsciLexer* lexerForExtension(const QString& suffix);

    static QsciAPIs* s_apis;
    static QsciLexerLua* s_lexLua;
    static QsciLexerCPP* s_lexCpp;
    static QsciLexerMatlab* s_lexMatlab;
    static QsciLexerJavaScript* s_lexJs;
    static QsciLexerHTML* s_lexHtml;

    QString m_dir;
    QString m_title;
    int m_findIndicator;
    QString m_findText;
    bool m_lastFindForward;
    // EditorPosition mPositionBeforeFind;

};

#endif // FILEEDITOR_H
