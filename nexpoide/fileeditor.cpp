#include "fileeditor.h"
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QFont>
#include <QFileDialog>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QSaveFile>
#include <QScrollBar>
#include <QKeyEvent>

#include <Qsci/qsciapis.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexermatlab.h>
#include <iostream>

//#include "application.h"
//#include "mainwindow.h"
//#include "settings.h"
//#include "editorfindbox.h"
//#include "ctypes.h"

// ------------------------------------------------------------
// Lexers and API
// ------------------------------------------------------------

//static QsciAPIs* gApi;
static QsciLexerLua* gLexLua;
static QsciLexerCPP* gLexCpp;
static QsciLexerMatlab* gLexMatlab;
static QsciLexerJavaScript* gLexJs;
static QsciLexerHTML* gLexHtml;
static bool gLexerDoneInit = false;

/*
QString getPreparedApiPath() {
    return gApp->applicationRootPath() + "/base/apis.dat";
}
*/

QFont getEditorFont()
{
    // Choose font
#ifdef Q_OS_MAC
    const char* defaultfont = "Menlo,12";
#else
    const char* defaultfont = "Consolas,10";
#endif
    QFont font;
    if (!font.fromString(defaultfont)) {
        font.setFixedPitch(true);
    }
    return font;
}

void setLexerStyle(QsciLexer* lex) {
    lex->setFont(getEditorFont());
    lex->setAutoIndentStyle(QsciScintilla::AiMaintain | QsciScintilla::AiOpening | QsciScintilla::AiClosing);
}

void initLexers()
{
    if (gLexerDoneInit) return;

    // Lexers
    gLexLua = new QsciLexerLua;
    gLexCpp = new QsciLexerCPP;
    gLexMatlab = new QsciLexerMatlab;
    gLexJs = new QsciLexerJavaScript;
    gLexHtml = new QsciLexerHTML;

    setLexerStyle(gLexLua);
    setLexerStyle(gLexCpp);
    setLexerStyle(gLexMatlab);
    setLexerStyle(gLexJs);
    setLexerStyle(gLexHtml);

    /*
    // Api
    gApi = new QsciAPIs(gLexLua);
    if (!gApi->loadPrepared(getPreparedApiPath())) {
        qDebug() << "initLexers: Failed to load prepared api in file" << getPreparedApiPath();
    }
    */

    QFont font = getEditorFont();
    QFont bold(font);
    bold.setBold(true);
    QFont italic(font);
    italic.setItalic(true);

    // 1=comment, 2=line comment
    gLexLua->setFont(italic, 1);
    gLexLua->setColor(Qt::gray, 1);
    gLexLua->setPaper(Qt::white, 1);

    // 2=multi-line comment
    gLexLua->setFont(italic, 2);
    gLexLua->setColor(Qt::gray, 2);
    gLexLua->setPaper(Qt::white, 2);

    // 4=number
    gLexLua->setColor(Qt::darkRed, 4);

    // 5=keyword
    gLexLua->setColor(Qt::blue, 6);
    gLexLua->setFont(bold, 5);

    // 6=double quote string
    gLexLua->setColor(Qt::darkGreen, 6);

    // 7=single quote string
    gLexLua->setColor(Qt::darkGreen, 7);

    // 8=double bracket string
    gLexLua->setColor(Qt::darkGreen, 8);
    gLexLua->setPaper(Qt::white, 8);

    // 9=preprocessor
    gLexLua->setPaper(Qt::cyan, 9);

    // 10=operator
    gLexLua->setColor(QColor(128, 0, 128), 10);

    // 11=identifier
    gLexLua->setColor(Qt::black, 11);

    // 12=Unclosed string
    gLexLua->setColor(Qt::darkGreen, 12);
    gLexLua->setPaper(Qt::white, 12);
    gLexLua->setFont(italic, 12);

    // 13=basic functions
    gLexLua->setPaper(Qt::white, 13);

    // 14=string, table, maths functions
    gLexLua->setPaper(Qt::white, 14);

    // 15=coroutine, i/o and system facilities
    gLexLua->setPaper(Qt::white, 15);

    // 16-19=user defined
    gLexLua->setPaper(Qt::red, 16);
    gLexLua->setPaper(Qt::green, 17);
    gLexLua->setPaper(Qt::yellow, 18);
    gLexLua->setPaper(Qt::gray, 19);

    // 20=label
    gLexLua->setPaper(QColor(0, 128, 128), 20);

    gLexerDoneInit = true;
}


QsciLexer* lexerForExtension(const QString& suffix) {
    initLexers();

    if (suffix.compare("html", Qt::CaseInsensitive) == 0 ||
        suffix.compare("htm", Qt::CaseInsensitive) == 0)
    {
        return gLexHtml;
    }
    else if (suffix.compare("vert", Qt::CaseInsensitive) == 0 ||
               suffix.compare("frag", Qt::CaseInsensitive) == 0 ||
               suffix.compare("geom", Qt::CaseInsensitive) == 0 ||
               suffix.compare("tess", Qt::CaseInsensitive) == 0 ||
               suffix.compare("glsl", Qt::CaseInsensitive) == 0 ||
               suffix.compare("cpp", Qt::CaseInsensitive) == 0 ||
               suffix.compare("c", Qt::CaseInsensitive) == 0 ||
               suffix.compare("h", Qt::CaseInsensitive) == 0 ||
               suffix.compare("hpp", Qt::CaseInsensitive) == 0)
    {
        return gLexCpp;
    }
    else if (suffix.compare("js", Qt::CaseInsensitive) == 0 ||
             suffix.compare("json", Qt::CaseInsensitive) == 0)
    {
        return gLexJs;
    }
    else if (suffix.compare("m", Qt::CaseInsensitive) == 0) {
        return gLexMatlab;
    }
    else if (suffix.compare("lua", Qt::CaseInsensitive) == 0 ||
             suffix.compare("nexpo", Qt::CaseInsensitive) == 0)
    {
        return gLexLua;
    }
    else {
        return 0;
    }
}

#if 0
// ------------------------------------------------------------
// Begin exports
// ------------------------------------------------------------

LUAEXPORT(const char* getLuaLexerStyleDescription(int style))
{
    initLexers();
    return tempString(gLexLua->description(style));
}

LUAEXPORT(void addApiEntry(const char* entry)) {
    initLexers();
    gApi->add(entry);
}

LUAEXPORT(void clearApi()) {
    initLexers();
    gApi->clear();
}

LUAEXPORT(void prepareApi()) {
    initLexers();
    gApi->prepare();
}

LUAEXPORT(bool savePreparedApi()) {
    initLexers();
    if (!gApi->isPrepared()) gApi->prepare();
    return gApi->savePrepared(getPreparedApiPath());
}

LUAEXPORT(void setLuaLexerFont(const char* desc, int style))
{
    initLexers();
    QFont font;
    if (!font.fromString(desc)) {
        warn(QString("setLuaLexerFont: bad font string: ") + desc);
        return;
    }

    gLexLua->setFont(font, style);
    if (style < 0) {
        gLexLua->setDefaultFont(font);
    }
}

LUAEXPORT(void setLuaLexerColor(const vec4* col, int style))
{
    initLexers();
    QColor color;
    color.setRgbF(col->x, col->y, col->z, col->w);
    gLexLua->setColor(color, style);
    if (style < 0) {
        gLexLua->setDefaultColor(color);
    }
}

LUAEXPORT(void setLuaLexerBgColor(const vec4* col, int style)) {
    initLexers();
    QColor color;
    color.setRgbF(col->x, col->y, col->z, col->w);
    gLexLua->setPaper(color, style);
    if (style < 0) {
        gLexLua->setDefaultPaper(color);
    }
}

LUAEXPORT(const char* getFontString(const char* family,
                                    float pt,
                                    bool italic,
                                    bool bold,
                                    bool underline,
                                    bool strikeout))
{
    QFont font = getEditorFont();
    if (family) font.setFamily(family);
    if (pt > 0) font.setPointSizeF(pt);
    font.setItalic(italic);
    font.setBold(bold);
    font.setUnderline(underline);
    font.setStrikeOut(strikeout);
    return tempString(font.toString());
}

// ------------------------------------------------------------
// End exports
// ------------------------------------------------------------

#endif

FileEditor::FileEditor(QWidget *parent)
    : QsciScintilla(parent)
{
    m_findIndicator = indicatorDefine(QsciScintilla::BoxIndicator);

    /*
    // Find widget
    mFindBox = new EditorFindBox;

    // Find next
    connect(mFindBox, &EditorFindBox::find, [=]() {
        findFirst();
    });

    // Find prev
    connect(mFindBox, &EditorFindBox::findPrev, this, &FileEditor::findPrev);

    // Highlight find text as it is typed
    connect(mFindBox, &EditorFindBox::textChanged, [=](QString) {


    });

    // Find box hidden
    connect(mFindBox, &EditorFindBox::hidden, [=]() {
        // Clear marked find text
        clearIndicatorRange(0, 0, lines()-1, lineLength(lines()-1), mFindIndicator);

        // Give focus to editor
        setFocus();
    });

    // Replace
    connect(mFindBox, &EditorFindBox::replace, [=]() {
        if (hasSelectedText()) {
            int lineFrom, indexFrom, lineTo, indexTo;
            getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
            setCursorPosition(lineFrom, indexFrom);
        }

        if (findFirst()) {
            replace(mFindBox->replaceText());
            findNext();
        }
    });

    // Replace all
    connect(mFindBox, &EditorFindBox::replaceAll, [=]() {
        beginUndoAction();

        // Save cursor position
        int line, index;
        getCursorPosition(&line, &index);

        // Save selection
        bool hadSelection = hasSelectedText();
        int lineFrom, indexFrom, lineTo, indexTo;
        if (hadSelection) {
            getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
        }

        // Replace from start
        setCursorPosition(0, 0);

        // Restore cursor position and selection if none found
        if (!findFirst(ForwardNoWrap)) {
            setCursorPosition(line, index);
            if (hadSelection) {
                setSelection(lineFrom, indexFrom, lineTo, indexTo);
            }
        }

        // Replace all
        do {
            replace(mFindBox->replaceText());
        } while (findNext());

        endUndoAction();
    });

    layout()->addWidget(mFindBox);
    mFindBox->hide();
    */

    // Default lexer
    installLexer("lua");
}

FileEditor::~FileEditor() {
    //qDebug() << "FileEditor destructor";
}

void FileEditor::highlightFindText(const QString& text)
{
    clearIndicatorRange(0, 0, lines()-1, lineLength(lines()-1), m_findIndicator);

    if (text.size() == 0) {
        int line, index;
        getCursorPosition(&line, &index);
        emit cursorPositionChanged(line, index);
        return;
    }

    // Save scroll position
    int vscroll = verticalScrollBar()->value();
    int hscroll = horizontalScrollBar()->value();

    bool hadSelection = hasSelectedText();
    int lineFrom, indexFrom, lineTo, indexTo;
    if (hadSelection) {
        getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
        setCursorPosition(lineFrom, indexFrom);
    }

    if (!find(text)) {
        if (hadSelection) setSelection(lineFrom, indexFrom, lineTo, indexTo);
        emit statusMessageChanged("No matches for " + text);
        return;
    }

    // Save selection of first result
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    int numMatches = 0;
    do {
        int nextLineFrom, nextIndexFrom, nextLineTo, nextIndexTo;
        getSelection(&nextLineFrom, &nextIndexFrom, &nextLineTo, &nextIndexTo);
        fillIndicatorRange(nextLineFrom, nextIndexFrom, nextLineTo, nextIndexTo, m_findIndicator);

        if (numMatches>0 && nextLineFrom == lineFrom && nextIndexFrom == indexFrom) break;
    } while (find(text) && numMatches++ < 10000);
    //qDebug() << "find: i=" << i;

    // Restore selection
    setSelection(lineFrom, indexFrom, lineTo, indexTo);

    // Restore original scroll position
    verticalScrollBar()->setValue(vscroll);
    horizontalScrollBar()->setValue(hscroll);

    // Ensure selection is visible
    ensureCursorVisible();

    emit statusMessageChanged(QString("%1 match%2 for %3")
                              .arg(numMatches)
                              .arg(numMatches == 1 ? "" : "es")
                              .arg(text));

}

void FileEditor::unhighlightFindText()
{
    highlightFindText(QString());
}

bool FileEditor::find(const QString& text, bool forward)
{
    m_findText = text;
    m_lastFindForward = forward;
    return findFirst(text,
                       false, //mFindBox->regularExpression(),
                       false, //mFindBox->caseSensitive(),
                       false, //mFindBox->wholeWordsOnly(),
                       true, // (mFindBox->wrap() && direction != ForwardNoWrap),
                       forward, //(direction == Forward || direction == ForwardNoWrap),         // forward
                       -1,           // line
                       -1,           // index in line
                       true,         // unfold folded text
                       true         // treat ( and ) as tagged sections in regexps
                     );
}

bool FileEditor::findNextForward()
{
    if (m_lastFindForward) {
        return findNext();
    } else {
        return find(m_findText, true);
    }
}

bool FileEditor::findNextBackward()
{
    // Move cursor to start of selection
    int lineFrom, indexFrom, lineTo, indexTo;
    getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
    setCursorPosition(lineFrom, indexFrom);

    // Search backward
    return find(m_findText, false);
}

void FileEditor::selectLines(int first, int last)
{
    // for some reason you have to specify one less than the first line
    setSelection(first-1, 0, last, -1);
    ensureLineVisible(first);
}

void FileEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        emit escapePressed();
    } else {
        QsciScintilla::keyPressEvent(e);
    }
}

/*
void FileEditor::showFindBox() {
    mFindBox->show();
    mFindBox->takeFocus();
}


void FileEditor::findNext()
{
    findFirst();
}

void FileEditor::findPrev()
{
    if (hasSelectedText()) {
        int lineFrom, indexFrom, lineTo, indexTo;
        getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
        setCursorPosition(lineFrom, indexFrom);
    }
    findFirst(Backward);
}
*/

void FileEditor::installLexer(const QString& suffix)
{
    // Get lexer for this file
    QsciLexer* lex = lexerForExtension(suffix);


    // Set lexer style and apply it to editor
    //lex->setParent(this);
    //lex->setFont(getEditorFont());
    //lex->setAutoIndentStyle(QsciScintilla::AiMaintain | QsciScintilla::AiOpening | QsciScintilla::AiClosing);
    //QSettings set;
    //lex->readSettings(set, "editor");
    //lex->writeSettings(set, "Script/editor");

    if (lex) {
        setLexer(lex);
        setMarginsBackgroundColor(lex->defaultPaper());
    } else {
        setLexer(0);
        setMarginsBackgroundColor(paper());
        setFont(getEditorFont());
    }

    // Editor settings
    setUtf8(true);
    setMarginsFont(getEditorFont());
    setMarginType(0, QsciScintilla::NumberMargin);
    setMarginLineNumbers(0, true);
    setMarginWidth(0, "99999");
    setMarginsForegroundColor(QColor("#ccc"));
    setTabWidth(2);
    setAutoIndent(true);
    setIndentationsUseTabs(false);
    setIndentationWidth(2);
    setIndentationGuides(true);
    setTabIndents(true);
    setIndentationGuidesForegroundColor(Qt::lightGray);
    setBraceMatching(QsciScintilla::SloppyBraceMatch);
    setMatchedBraceBackgroundColor(QColor::fromRgbF(.678, .847, .602, 1));
    setUnmatchedBraceBackgroundColor(QColor::fromRgbF(.980, .502, .447, 1));
    setWrapMode(QsciScintilla::WrapWord);
    setWrapIndentMode(QsciScintilla::WrapIndentSame);
    setFrameShape(QFrame::NoFrame);
    //setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    //this->setLineWidth(2);
    //this->setContentsMargins(2, 0, 0, 0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setEolMode(QsciScintilla::EolUnix);
    setAutoCompletionSource(QsciScintilla::AcsAPIs);
    setEdgeMode(QsciScintilla::EdgeLine);
    //setEdgeMode(QsciScintilla::EdgeBackground);
    setEdgeColumn(80);
    setEdgeColor(QColor::fromRgb(255, 238, 214));

}

bool FileEditor::open(const QString& path)
{
    // Check file info
    QFileInfo info(path);
    if (!info.isFile()) return false;

    // Read the file
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) return false;
    if (!read(&file)) return false;

    setFileInfo(&info);

    // Set modified to false (it seems to be true intially?)
    setModified(false);
    return true;
}

QString FileEditor::title() const
{
    if (m_path.size() == 0) {
        return "Untitled";
    } else {
        QFileInfo info(m_path);
        return info.fileName();
    }
}

void FileEditor::setFileInfo(const QFileInfo* info)
{
    // Set lexer
    installLexer(info->suffix());

    // Set members
    m_path = info->canonicalFilePath();
    emit titleChanged(title());
}

bool FileEditor::saveFile()
{
    if (m_path.size() == 0) return false;

    QSaveFile file(m_path);
    file.setDirectWriteFallback(true);  // fallback to not using a temp file if we can't create one
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QByteArray data = text().toUtf8();
    qint64 written = file.write(data);
    if (written != data.size() || !file.commit()) {
        std::cerr << "Failed to save " << m_path.toUtf8().data() << std::endl;
        return false;
    } else {
        setModified(false);
        QFileInfo info(m_path);
        setFileInfo(&info);
        return true;
    }
}

bool FileEditor::saveFileAs(const QString& newPath)
{
    m_path = newPath;
    return saveFile();
}
