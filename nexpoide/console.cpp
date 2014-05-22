#include "console.h"
#include <QTextBlock>
#include <QTextDocument>
#include <QTextCursor>
#include <QClipboard>
#include <QApplication>
#include <iostream>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_commandLineReady(false)
{
    QFont f;
#ifdef Q_OS_LINUX
    f.setFamily("Monospace");
    f.setPointSize(12);
#elif defined(Q_OS_MAC)
    f.setFamily("Menlo");
    f.setPointSize(12);
#else
    f.setFamily("Consolas");
    f.setPointSize(12);
#endif
    setFont(f);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setUndoRedoEnabled(false);
    //setLineWrapMode(QPlainTextEdit::NoWrap);
    //setWordWrapMode(QTextOption::NoWrap);
    setBackgroundVisible(false);
    setFrameStyle(QFrame::NoFrame);
    setTabStopWidth(40);
    setAcceptDrops(false);
    setPrefix("Nexpo> ");
    setPrefixColor(QColor(30, 128, 50));
    f.setBold(true);
    setPrefixFont(f);

    m_errorColor = QColor(200, 0, 0);

    prepareCommandLine();
}

void Console::setPrefix(const QString& prefix) {
    m_prefix = prefix;
}

void Console::setPrefixFont(const QFont& f) {
    m_prefixFont = f;
}

void Console::setPrefixColor(const QColor& c) {
    m_prefixColor = c;
}

bool Console::inCommandLine() const
{
    return document()->blockCount()-1 == textCursor().blockNumber() && textCursor().positionInBlock() >= m_prefix.length();
}

void Console::moveToEndOfCommandLine()
{
    QTextCursor cur(document()->lastBlock());
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.setCharFormat(QTextCharFormat());
    setTextCursor(cur);
}

void Console::cls() {
    clear();
    prepareCommandLine();
}

void Console::prepareCommandLine()
{
    QTextCursor cur(document()->lastBlock());
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.insertBlock();

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(m_prefixColor));
    fmt.setFont(m_prefixFont);
    cur.setCharFormat(fmt);
    cur.insertText(m_prefix);
    cur.setCharFormat(QTextCharFormat());
    setTextCursor(cur);
}

void Console::processCommand()
{
    QString inputString = document()->lastBlock().text();
    if (!inputString.startsWith(m_prefix)) {
        std::cerr << "Error: last block of console did not start with prefix" << std::endl;
    }

    inputString.remove(0, m_prefix.length());
    if (!inputString.trimmed().isEmpty()) {
        if (m_history.size() == 0 || m_history.back() != inputString) {
            m_history.append(inputString);
        }
    }
    m_historyPos = -1;
    inputString = inputString.trimmed();

    prepareCommandLine();

    emit command(inputString);
}

// {true, 1.345778, 'hello world', {myfunc = print}, ffi.new('char[3]'), coroutine.create(print)}

void Console::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) // process command
    {
      processCommand();
      return;
    }




  if (inCommandLine())
  {
    // clear selection that spans multiple blocks (or prefix characters) (would overwrite previous command lines):
    QTextCursor cur = textCursor();
    if (cur.hasSelection())
    {
      if (document()->findBlock(cur.selectionStart()) != document()->findBlock(cur.selectionEnd()) || // spans multiple blocks (including command line)
          cur.selectionStart()-cur.block().position() < m_prefix.length() || // spans prefix
          cur.selectionEnd()-cur.block().position() < m_prefix.length() ) // spans prefix
      {
        cur.clearSelection();
        if (cur.positionInBlock() < m_prefix.length())
          cur.setPosition(cur.block().position()+m_prefix.length());
        setTextCursor(cur);
      }
    }
    if (cur.positionInBlock() == m_prefix.length())
    {
      cur.setCharFormat(QTextCharFormat()); // make sure we don't pick up format from prefix
      setTextCursor(cur);
    }
    // react to keystroke:
    if (event->matches(QKeySequence::MoveToPreviousLine)) // history up
    {

      if (m_history.isEmpty() || m_historyPos >= m_history.size()-1)
        return;
      ++m_historyPos;
      int index = m_history.size()-m_historyPos-1;
      QTextCursor cur(document()->lastBlock());
      cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_prefix.length());
      cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      cur.removeSelectedText();
      cur.setCharFormat(QTextCharFormat());
      cur.insertText(m_history.at(index));
      setTextCursor(cur);

    } else if (event->matches(QKeySequence::MoveToNextLine)) // history down
    {

      if (m_history.isEmpty() || m_historyPos <= 0)
        return;
      --m_historyPos;
      int index = m_history.size()-m_historyPos-1;
      QTextCursor cur(document()->lastBlock());
      cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_prefix.length());
      cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
      cur.removeSelectedText();
      cur.setCharFormat(QTextCharFormat());
      cur.insertText(m_history.at(index));
      setTextCursor(cur);
    } else if (event->matches(QKeySequence::Paste)) // paste text, do it manually to remove text char formatting and newlines
    {
      QString pasteText = QApplication::clipboard()->text();
      pasteText.replace("\n", "").replace("\r", "");
      cur.setCharFormat(QTextCharFormat());
      cur.insertText(pasteText);
      setTextCursor(cur);
    } else if (event->key() == Qt::Key_Backspace || event->matches(QKeySequence::MoveToPreviousChar)) // only allow backspace if we wouldn't delete last char of prefix, similar left arrow
    {
      if (cur.positionInBlock() > m_prefix.length())
        QPlainTextEdit::keyPressEvent(event);
    } else if (event->matches(QKeySequence::MoveToStartOfLine) || event->key() == Qt::Key_Home) {
        // Don't move past prefix when pressing home
        // OSX treats the home key as MoveToStartOfDocument, so including the key code here too
        cur.movePosition(QTextCursor::PreviousCharacter,
                         event->modifiers() & Qt::ShiftModifier ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor,
                         cur.positionInBlock() - m_prefix.length());
        cur.setCharFormat(QTextCharFormat());
        setTextCursor(cur);
    } else if (event->key() == Qt::Key_Escape) {
        QTextCursor cur(document()->lastBlock());
        cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_prefix.length());
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cur.removeSelectedText();
        setTextCursor(cur);
    } else if (!event->matches(QKeySequence::Close) &&
               !event->matches(QKeySequence::New) &&
               !event->matches(QKeySequence::Open) &&
               !event->matches(QKeySequence::Preferences) &&
               !event->matches(QKeySequence::Bold) &&
               !event->matches(QKeySequence::Italic) &&
               !event->matches(QKeySequence::InsertLineSeparator) &&
               !event->matches(QKeySequence::InsertParagraphSeparator) &&
               !event->matches(QKeySequence::Redo) &&
               !event->matches(QKeySequence::Undo) &&
               !event->matches(QKeySequence::DeleteStartOfWord))
    {
      QPlainTextEdit::keyPressEvent(event);
    }
  } else // cursor position not in command line
  {
    if (event->matches(QKeySequence::MoveToEndOfDocument) ||
        event->matches(QKeySequence::MoveToEndOfBlock) ||
        event->matches(QKeySequence::MoveToEndOfLine) ||
        event->matches(QKeySequence::MoveToStartOfDocument) ||
        event->matches(QKeySequence::MoveToStartOfBlock) ||
        event->matches(QKeySequence::MoveToStartOfLine) ||
        //event->matches(QKeySequence::MoveToNextLine) ||
        event->matches(QKeySequence::MoveToNextWord) ||
        event->matches(QKeySequence::MoveToNextChar) ||
        //event->matches(QKeySequence::MoveToPreviousLine) ||
        event->matches(QKeySequence::MoveToPreviousWord) ||
        event->matches(QKeySequence::MoveToPreviousChar) ||
        event->matches(QKeySequence::SelectAll) ||
        event->matches(QKeySequence::SelectEndOfDocument) ||
        event->matches(QKeySequence::SelectEndOfBlock) ||
        event->matches(QKeySequence::SelectEndOfLine) ||
        event->matches(QKeySequence::SelectStartOfDocument) ||
        event->matches(QKeySequence::SelectStartOfBlock) ||
        event->matches(QKeySequence::SelectStartOfLine) ||
        event->matches(QKeySequence::SelectNextLine) ||
        event->matches(QKeySequence::SelectNextWord) ||
        event->matches(QKeySequence::SelectNextChar) ||
        event->matches(QKeySequence::SelectPreviousLine) ||
        event->matches(QKeySequence::SelectPreviousWord) ||
        event->matches(QKeySequence::SelectPreviousChar) ||
        event->matches(QKeySequence::Copy) ||
           event->key() == Qt::Key_Shift ||
           event->key() == Qt::Key_Alt ||
           event->key() == Qt::Key_Control ||
           event->key() == Qt::Key_Meta
       )
    {


        QPlainTextEdit::keyPressEvent(event);
    } else {
        // Place cursor in command line
        moveToEndOfCommandLine();
        QPlainTextEdit::keyPressEvent(event);
    }
  }
}

void Console::print(const QString& str)
{
    printWithFormat(str);
}

void Console::printWithFormat(const QString& str, const QTextCharFormat& fmt)
{
    const QString escape = QStringLiteral("\x1b[");

    QTextCursor cur(document()->lastBlock());
    cur.movePosition((QTextCursor::StartOfBlock));
    cur.setCharFormat(fmt);

    int startIndex = 0;
    while(true) {
        int escapeIndex = str.indexOf(escape, startIndex);
        if (escapeIndex == -1) {
            // no more escape codes found, output rest of string
            cur.insertText(str.mid(startIndex, str.length()-startIndex));
            break;
        }

        // escape code found, output everything up to it
        cur.insertText(str.mid(startIndex, escapeIndex-startIndex));

        // look for 'm', the termination character for graphic escape codes
        int termIndex = str.indexOf('m', escapeIndex);

        // if didn't find termination code, jump over escape sequence and loop
        if (termIndex == -1) {
            startIndex = escapeIndex + escape.size();
            continue;
        }

        // found termination code, set startIndex for next loop iteration
        startIndex = termIndex + 1;

        // extract payload: should be one or more numbers separated by semicolons
        int formatCodeStart = escapeIndex + escape.size();

        // read format codes
        while (formatCodeStart < termIndex) {
            // Look for digits
            int formatCodeEnd = formatCodeStart;
            while (str[formatCodeEnd].isDigit() && formatCodeEnd < termIndex) {
                formatCodeEnd++;
            }

            // abort if we overran the escape sequence
            if (formatCodeEnd > termIndex) break;

            // convert to number
            QStringRef formatCodeString = str.midRef(formatCodeStart, formatCodeEnd-formatCodeStart);
            bool ok = false;
            int formatCode = formatCodeString.toUInt(&ok);
            if (!ok) break;
            applyFormatCode(cur, formatCode);

            // if a semicolon follows, loop again, otherwise we're done
            if (str[formatCodeEnd] != ';') break;
            formatCodeStart = formatCodeEnd + 1;
        }
    }

    if (!str.endsWith("\n")) {
        cur.insertBlock();
    }
    setTextCursor(cur);
    moveToEndOfCommandLine();
}

void Console::printError(const QString& str)
{
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(m_errorColor));
    printWithFormat(str, fmt);
}

// These are the same as those of Terminal.app
static const QBrush ansiColorsNormal[] = {
    QColor(0, 0, 0),
    QColor(194, 54, 33),
    QColor(37, 188, 36),
    QColor(173, 173, 39),
    QColor(73, 46, 225),
    QColor(211, 56, 211),
    QColor(51, 187, 200),
    QColor(203, 204, 205),
};

// These are 0.75x the brightness of those above, for use on a white background
static const QBrush ansiColorsDark[] = {
    QColor(0, 0, 0),
    QColor(146, 41, 25),
    QColor(28, 141, 27),
    QColor(130, 130, 29),
    QColor(55, 35, 169),
    QColor(158, 42, 158),
    QColor(38, 140, 150),
    QColor(152, 153, 154),
};


static const QBrush ansiColorsBright[] = {
    QColor(129, 131, 131),
    QColor(252, 57, 31),
    QColor(49, 231, 34),
    QColor(234, 236, 35),
    QColor(88, 51, 225),
    QColor(249, 53, 248),
    QColor(20, 240, 240),
    QColor(255, 255, 255), // QColor(233, 235, 235),
};

void Console::applyFormatCode(QTextCursor& cur, unsigned int code)
{
    QTextCharFormat fmt = cur.charFormat();

    switch(code) {
    // reset format
    case 0: {
        cur.setCharFormat(QTextCharFormat()); return;
    }

    // font styles
    case 1: fmt.setFontWeight(QFont::Bold); break;
    case 2: fmt.setFontWeight(QFont::Light); break;
    case 3: fmt.setFontItalic(true); break;
    case 4: fmt.setFontUnderline(true); break;
    case 9: fmt.setFontStrikeOut(true); break;
    case 22: fmt.setFontWeight(QFont::Normal); break;
    case 23: fmt.setFontItalic(false); break;
    case 24: fmt.setFontUnderline(false); break;
    case 29: fmt.setFontStrikeOut(false); break;

    // low foreground color
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
       fmt.setForeground(ansiColorsDark[code-30]); break;

    // default foreground color
    case 39: fmt.setForeground(Qt::black); break;

    // low background color
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
        fmt.setBackground(ansiColorsDark[code-40]); break;

    // default background color
    case 49: fmt.setBackground(Qt::white); break;

    // overline style
    case 53: fmt.setFontOverline(true); break;
    case 55: fmt.setFontOverline(false); break;

    // high foreground color
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
        fmt.setForeground(ansiColorsBright[code-90]); break;

    // high background color
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
        fmt.setBackground(ansiColorsBright[code-100]); break;

    default: return;
    }

    cur.setCharFormat(fmt);
}

