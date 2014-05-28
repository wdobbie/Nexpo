#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileeditor.h"
#include "outputredirector.h"
#include "updatechecker.h"
#include "aboutform.h"
#include "application.h"
#include "scriptstatuswidget.h"
#include "sliderwithspinner.h"
#include "linkbutton.h"
#include "timeseriesplot.h"

#include <QFileDialog>
#include <QSettings>
#include <QStringList>
#include <QProcess>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProxyStyle>
#include <QStringList>
#include <QDateTime>
#include <QMimeData>
#include <QCompleter>
#include <iostream>


static const char kControlPrefix = '\x05';
// Stylesheet for QToolButtons that appear as hyperlinks

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scriptProcess(0)
    , m_updateChecker(0)
    , m_currentEditor(0)
    , m_scriptStatusWidget(0)
    , m_helpData(0)
{
    ui->setupUi(this);

    // Clear dummy tabs from form designer
    ui->openTabs->clear();

    // Accept file drops from OS
    setAcceptDrops(true);

    // Redirect stdout and sterr to console
    m_stdOutRedirector = new OutputRedirector(1);
    m_stdErrRedirector = new OutputRedirector(2);

    connect(m_stdOutRedirector, &OutputRedirector::output, ui->console, &Console::print);
    connect(m_stdErrRedirector, &OutputRedirector::output, ui->console, &Console::printError);

    // Add left spacer
    QWidget* spacer1 = new QWidget(this);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    spacer1->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->mainToolBar->insertWidget(ui->actionConsole, spacer1);

    // Add status widget
    m_scriptStatusWidget = new ScriptStatusWidget(this);
    QAction* scriptStatusAction = ui->mainToolBar->insertWidget(ui->actionConsole, m_scriptStatusWidget);
    scriptStatusAction->setVisible(false);
    connect(this, &MainWindow::scriptRunningChanged, scriptStatusAction, &QAction::setVisible);
    connect(this, &MainWindow::scriptElapsedChanged, m_scriptStatusWidget, &ScriptStatusWidget::setElapsed);

    // Add right spacer
    QWidget* spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    spacer2->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->mainToolBar->insertWidget(ui->actionConsole, spacer2);

    // Restore window state
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

    // Extra options for tab widget (can't set from form designer)
    ui->openTabs->tabBar()->setExpanding(true);
    ui->openTabs->tabBar()->setDrawBase(false);

    // Hide find and goto line widgets (can't do this in form designer)
    ui->findWidget->hide();
    ui->gotoLineWidget->hide();

    // Hide help dock widget
    ui->helpDockWidget->hide();

    // Init editor static variables (lexers, api)
    FileEditor::init(this);

    // Load help data (must be done after FileEditor::init)
    loadHelpData();

    // Connect editor changed signal
    connect(this, &MainWindow::currentEditorChanged, this, &MainWindow::onCurrentEditorChanged);

    // Populate recent files menu
    createRecentFileActions();

    // Welcome widget
    setupWelcomeWidget();

    // Set initial state of central widget
    showCentralWidget();

    // Create process to run scripts
    startScriptProcess();

    // Connect console input commands to script stdinput
    connect(ui->console, &Console::command, this, &MainWindow::writeToScript);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_helpData;
}

// Returns true if all tabs close, false if user cancelled
bool MainWindow::closeAllTabs()
{
    while (ui->openTabs->count() > 0) {
        if (!closeEditor(qobject_cast<FileEditor*>(ui->openTabs->currentWidget()))) {
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // TODO: investigate why this function is called twice when using command-Q on OS X
    // It's only called once when clicking the close window button


    QSettings settings;

    // Ask to terminate running script
    if (m_scriptRunning) {
        QMessageBox msg;
        QFileInfo info(m_runningScriptPath);
        msg.setText(info.fileName() + " is still running");
        msg.setInformativeText("Stop it?");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Yes);
        switch(msg.exec()) {
        case QMessageBox::Yes:
            stopCurrentScript();
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            event->ignore();
            return;
        default:
            // should not be reached
            break;
        }
    }

    saveSession();

    // Close all tabs
    if (!closeAllTabs()) {
        event->ignore();
        return;
    }

    // It's either stopped or we're pretending it is
    m_scriptRunning = false;

    // Save window arrangement
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
}

void MainWindow::focusCurrentEditor()
{
    if (currentEditor()) {
        currentEditor()->setFocus();
    }
}

void MainWindow::hideOneInterfaceElement()
{
   // Hide one interface element
    QWidget* widgets[] = {
        ui->gotoLineWidget,
        ui->findWidget,
        ui->helpDockWidget,
        ui->consoleDockWidget,
        ui->controlsDockWidget,
    };

    int n = sizeof(widgets)/sizeof(widgets[0]);
    for (int i=0; i<n; i++) {
        if (widgets[i]->isVisible()) {
            widgets[i]->hide();
            return;
        }
    }

}

void MainWindow::addEditor(FileEditor* editor)
{
    // Update currentEditor when editor gets focus
    connect(editor, &FileEditor::gotFocus, this, &MainWindow::updateCurrentEditor);

    // The status bar connection must be queued, or else there is a crash during what looks like scintilla's paint event
    connect(editor, &QsciScintilla::cursorPositionChanged, this, &MainWindow::updateStatusBar, Qt::QueuedConnection);
    connect(editor, &QsciScintilla::modificationChanged, this, &MainWindow::updateTabTitles);
    connect(editor, &FileEditor::titleChanged, this, &MainWindow::updateTabTitles);
    connect(editor, &FileEditor::escapePressed, this, &MainWindow::hideOneInterfaceElement);

    // clear highlighted text when find box closes
    connect(ui->findLineEdit, &FindLineEdit::hidden, editor, &FileEditor::unhighlightFindText);

    // status message
    connect(editor, &FileEditor::statusMessageChanged, this, &MainWindow::editorStatusMessageChanged, Qt::QueuedConnection);

    // enable/disable copy/cut actions
    connect(editor, &FileEditor::copyAvailable, [=](bool avail){
        if (qobject_cast<FileEditor*>(sender()) == m_currentEditor) {
            ui->actionCopy->setEnabled(avail);
            ui->actionCut->setEnabled(avail);
        }
    });

    int index = ui->openTabs->addTab(editor, editor->title());
    ui->openTabs->setCurrentIndex(index);
}

void MainWindow::loadFile(const QString& path)
{
    if (path.size() == 0) return;

    QFileInfo info(path);
    if (!info.exists()) return;

    // Check if it's already open
    for (int i=0; i<ui->openTabs->count(); i++) {
        FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->widget(i));
        if (editor != 0 && editor->path() == info.absoluteFilePath()) {
            // File already open in editor, switch to it and return
            ui->openTabs->setCurrentIndex(i);
            showCentralWidget();
            return;
        }
    }

    FileEditor* editor = new FileEditor(this);

    if (!editor->open(path)) {
        std::cerr << "Failed to open " << path.toUtf8().constData() << std::endl;
        delete editor;
        return;
    }

    addEditor(editor);
    addRecentFile(editor->path());
    showCentralWidget();
    saveSession();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        foreach (QUrl url, mimeData->urls()) {
            loadFile(url.toLocalFile());
        }
    }
    event->acceptProposedAction();
}

void MainWindow::on_actionNew_triggered()
{
    // Get next free "Untitled #" number
    QString defaultName = QStringLiteral("Untitled");
    unsigned int num = 1;
    for (int i=0; i<ui->openTabs->count(); i++) {
        FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->widget(i));
        if (!editor) continue;
        if (editor->title().compare(defaultName, Qt::CaseInsensitive) == 0) {
            num = qMax(num, 2U);
        } else if (editor->title().startsWith(defaultName, Qt::CaseInsensitive)) {
            num = qMax(num, editor->title().mid(defaultName.size()).toUInt()+1);
        }
    }

    QString newName = (num == 1) ? defaultName : QString("%1 %2").arg(defaultName).arg(num);

    FileEditor* editor = new FileEditor(this);
    editor->setTitle(newName);
    addEditor(editor);
    editor->insert(
        "require 'Nexpo'\n"
        "\n"
        "c = circle()\n"
        "\n"
        "function update()\n"
        "  draw(c)\n"
        "end\n"
        "\n"
        "start()\n");
    editor->setModified(false);     // so can close tab without resistance
}


void MainWindow::openFileAction()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        loadFile(action->data().toString());
    }
}

void MainWindow::updateTabTitles()
{
    for (int i=0; i<ui->openTabs->count(); i++) {
        FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->widget(i));
        if (editor == 0) return;

        QString title = editor->title();
        if (editor->isModified()) title += " •";
        ui->openTabs->setTabText(i, title);
    }
}

void MainWindow::writeToScript(const QByteArray& cmd)
{
    if (m_scriptProcess && m_scriptRunning) {
        m_scriptProcess->write(cmd);
        m_scriptProcess->write(QByteArrayLiteral("\n"));
    } else {
        ui->console->printError("No script is running\n");
    }
}

void MainWindow::updateStatusBar()
{
    if (currentEditor() == 0) {
        ui->statusBar->clearMessage();
    } else {
        if (!ui->findWidget->isVisible() || ui->findLineEdit->text().size() == 0) {
            int line, index;
            currentEditor()->getCursorPosition(&line, &index);
            ui->statusBar->showMessage(QString("Line %1, Column %2").arg(line+1).arg(index+1));
        }
    }
}

void MainWindow::editorStatusMessageChanged(const QString &msg, int timeout)
{
    if (currentEditor() == sender()) {
        ui->statusBar->showMessage(msg, timeout);
    }
}

static bool isNewerVersion(const QString& version) {
    QStringList current = QString(APP_VERSION).split(".");
    QStringList latest = version.split(".");

    while (current.size() < latest.size()) {
        current.append("0");
    }

    while (latest.size() < current.size()) {
        latest.append("0");
    }

    for (int i=0; i<current.size(); i++)
    {
        if (current[i].toUInt() < latest[i].toUInt()) return true;
    }

    return false;
}

void MainWindow::showLatestVersion(const QString &version, const QString &urlString, const QString& message)
{
    if (isNewerVersion(version)) {
        QUrl url(urlString);
        QString link = urlString;
        if (url.isValid()) {
            link = QString("<a href=\"%1\">%2</a>").arg(urlString).arg(url.fileName());
        }
        ui->updateVersion->setText(version);
        ui->updateUrl->setText(link);
        ui->updateUrl->setToolTip(urlString);
        ui->updateMessage->setText(message);
        ui->updateMessage->setVisible(message.size() > 0);
        ui->updateAvailableWidget->show();
    }
    m_updateChecker->deleteLater();
    m_updateChecker = 0;
}

void MainWindow::createRecentFileActions()
{
    for (int i=0; i<MaxRecentFiles; i++) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], &QAction::triggered, this, &MainWindow::openFileAction);
        ui->menuOpen_Recent->addAction(recentFileActions[i]);
    }
    ui->menuOpen_Recent->addSeparator();
    ui->menuOpen_Recent->addAction(ui->actionClear_Recent_Files);
    updateRecentFileActions();
}

bool MainWindow::closeEditor(FileEditor* editor)
{
    if (editor == 0) return false;

    // Get user consent if it has unsaved changes
    if (editor->isModified()) {
        QMessageBox msg(this);
        msg.setText(editor->title() + " has been modified.");
        msg.setInformativeText("Do you want to save your changes?");
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Save);
        switch(msg.exec()) {
        case QMessageBox::Save:
            if (!editorSave(editor)) {
                return false;
            }
            break;
        case QMessageBox::Discard:
            break;
        case QMessageBox::Cancel:
            return false;
        default:
            // should never be reached
            return false;
        }
    }

    // Close tab if it's in a tab
    int tabIndex = ui->openTabs->indexOf(editor);
    if (tabIndex > 0) {
        ui->openTabs->removeTab(tabIndex);
    }

    // Unset currentEditor if it's the current editor
    if (m_currentEditor == editor) {
        m_currentEditor = 0;
        emit currentEditorChanged(0);
    }

    // Delete
    delete editor;

    return true;
}

void MainWindow::on_openTabs_tabCloseRequested(int index)
{
    closeEditor(qobject_cast<FileEditor*>(ui->openTabs->widget(index)));
}

void MainWindow::on_action_Open_triggered()
{
    QSettings settings;
    QString openPath = settings.value("lastOpenPath", nexpoPath() + "/examples").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open", openPath, "Nexpo files (*.nexpo *.lua);;All files (*.*)");
    if (fileName.size() == 0) return;
    QFileInfo info(fileName);
    if (info.exists()) settings.setValue("lastOpenPath", info.absolutePath());
    loadFile(fileName);
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    // Delete current entries in welcome screen
    while (ui->recentFilesContainer->layout()->count() > 0) {
        if (QLayoutItem* item = ui->recentFilesContainer->layout()->takeAt(0)) {
            QWidget* widget = item->widget();
            if (widget != 0 && widget->objectName() != QStringLiteral("restoreButton")) {
                delete widget;
            }
        }
    }

    // Update actions and create buttons
    QFileIconProvider ip;
    for (int i=0; i<numRecentFiles; i++) {
        QFileInfo info(files[i]);
        LinkButton* btn = new LinkButton(ui->recentFilesContainer);

        recentFileActions[i]->setVisible(true);
        recentFileActions[i]->setText(files[i]);
        recentFileActions[i]->setData(files[i]);

        btn->setDefaultAction(recentFileActions[i]);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setCursor(Qt::PointingHandCursor);

        if (info.exists()) {
            recentFileActions[i]->setEnabled(true);
            recentFileActions[i]->setToolTip("Open " + info.fileName());
            btn->setIcon(ip.icon(info));
        } else {
            recentFileActions[i]->setDisabled(true);
            recentFileActions[i]->setToolTip(info.fileName() + " not found");
            btn->setIcon(QProxyStyle().standardIcon(QStyle::SP_FileIcon));
        }


        ui->recentFilesContainer->layout()->addWidget(btn);
    }

    // Hide unused actions (in menu)
    for (int i=numRecentFiles; i<MaxRecentFiles; i++) {
        recentFileActions[i]->setVisible(false);
    }

    // Show "Clear Recent Files" if we have some
    ui->actionClear_Recent_Files->setVisible(numRecentFiles>0);
}

void MainWindow::addRecentFile(const QString& path)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(path);
    files.prepend(path);
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    settings.setValue("recentFileList", files);
    updateRecentFileActions();
    saveSession();
}

void MainWindow::saveSession()
{
    QSettings settings;

    // Save open tab list as last session
    QStringList session;
    for (int i=0; i<ui->openTabs->count(); i++) {
        FileEditor* editor = qobject_cast<FileEditor*>(ui->openTabs->widget(i));
        if (editor && editor->path().size()) {
            session.append(editor->path());
        }
    }
    if (session.count() > 0) {
        settings.setValue("lastSession", session);
        settings.setValue("lastSessionTime", QDateTime::currentDateTime());
        settings.setValue("lastSessionCurrentFile", currentEditor() ? currentEditor()->path() : QString());
    }
}

// TODO: should be tristate: saved, not saved, cancelled?
// So that the cancelled signal can be bubbled up to closeEvent
bool MainWindow::editorSaveAs(FileEditor* editor)
{
    if (editor == 0) return false;


    QString savePath = editor->path();
    QSettings settings;
    std::cerr << settings.value("lastSavePath").toString().toUtf8().constData() << std::endl;
    bool isNexpoExtension = true;
    if (savePath.size()) {
        // Check file extension
        QFileInfo info(savePath);
        isNexpoExtension = (info.suffix().compare("nexpo", Qt::CaseInsensitive) == 0);
    } else {
        QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        savePath = settings.value("lastSavePath", defaultPath).toString() + "/" + editor->title();
    }

    QString fileName = QFileDialog::getSaveFileName(
                this,
                "Save As",
                savePath,
                isNexpoExtension ? "Nexpo files (*.nexpo *.lua);;All files (*.*)" : QString());

    if (fileName.size() == 0) return false;

    QFileInfo info(fileName);
    settings.setValue("lastSavePath", info.absolutePath());

    if (!editor->saveFileAs(fileName)) {
        std::cerr << "Failed to save " << info.absoluteFilePath().toUtf8().constData() << std::endl;
        return false;
    } else {
        addRecentFile(editor->path());
        return true;
    }
}

bool MainWindow::editorSave(FileEditor* editor)
{
    if (editor == 0) return false;

    if (editor->path().size() == 0) {
        return editorSaveAs(editor);
    }

    if (!editor->saveFile()) {
        std::cerr << "Failed to save " << editor->path().toUtf8().constData() << std::endl;
        return false;
    } else {
        addRecentFile(editor->path());
        return true;
    }
}

void MainWindow::on_action_Save_triggered()
{
    editorSave(currentEditor());
}

void MainWindow::on_actionSave_As_triggered()
{
    editorSaveAs(currentEditor());
}



void MainWindow::startScriptProcess()
{
    m_scriptProcess = 0;

    // Get path to lua binary
    QSettings settings;
#ifdef Q_OS_MAC
    QString playerSubPath = QStringLiteral("/NexpoPlayer.app/Contents/MacOS/NexpoPlayer");
#elif defined(Q_OS_WIN)
    QString playerSubPath = QStringLiteral("/NexpoPlayer.exe");
#elif defined(Q_OS_LINUX)
    QString playerSubPath = QStringLiteral("/NexpoPlayer");
#else
#error Unsupported OS
#endif

    QString playerPath = settings.value("playerPath", nexpoPath() + playerSubPath).toString();
    if (playerPath.size() == 0) {
        std::cerr << "Error: missing nexpo player binary path" << std::endl;
        return;
    }

    // TODO: deal with existing process
    m_scriptProcess = new QProcess();

    // Connect process stderr and stdout to console
    connect(m_scriptProcess, &QProcess::readyReadStandardError, this, &MainWindow::scriptProcessStderrReady);
    connect(m_scriptProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::scriptProcessStdoutReady);

    // Auto start a new process when existing one finishes
    connect(m_scriptProcess, &QProcess::stateChanged, this, &MainWindow::scriptProcessStateChanged);

    // Start process
    m_scriptProcess->start(playerPath);
}

void MainWindow::setFrameInfo(const QByteArray& elapsed)
{
    m_scriptElapsed = elapsed.toDouble();
    emit scriptElapsedChanged(m_scriptElapsed);
}

void MainWindow::handleControlCommand(QByteArray cmd)
{
    int opLen = cmd.indexOf(' ');
    if (opLen < 0) opLen = cmd.size();
    QByteArray op = cmd.left(opLen);

    const int maxParams = 6;
    int numParams=0;
    QByteArray params[maxParams];
    for (int index=opLen+1; index < cmd.size() && numParams < maxParams; numParams++)
    {
        int nextSpace = cmd.indexOf(' ', index);
        if (nextSpace == -1) nextSpace = cmd.size();
        int paramLen = nextSpace - index;
        if (paramLen == 0) break;
        params[numParams] = cmd.mid(index, paramLen);
        index = nextSpace + 1;
    }

    if (op == QByteArrayLiteral("numbervalue")) {
        controls_setNumberValue(params[0], params[1]);
    } else if (op == QByteArrayLiteral("stringvalue")) {
        controls_setStringValue(params[0], QByteArray::fromHex(params[1]));
    } else if (op == QByteArrayLiteral("slider")) {
        controls_createSlider(params[0], params[1], params[2], params[3]);
    } else if (op == QByteArrayLiteral("checkbox")) {
        controls_createCheckbox(params[0], params[1]);
    } else if (op == QByteArrayLiteral("timeseriesplot")) {
        controls_createTimeSeriesPlot(params[0]);
    } else if (op == QByteArrayLiteral("frameinfo")) {
         setFrameInfo(params[0]);
    } else if (op == QByteArrayLiteral("help")) {
        showHelp(params[0]);
    }
}

void MainWindow::clearControls()
{
    while (ui->controlsLayout->count() > 0) {
        delete ui->controlsLayout->itemAt(0)->widget();
    }
    ui->controlsDockWidget->hide();
}


void MainWindow::sendControlCommand(const QByteArray &op, const QByteArray &param1, const QByteArray &param2)
{
    QByteArray cmd;
    cmd.reserve(3+op.size()+param1.size()+param2.size());
    cmd.append(kControlPrefix);
    cmd.append(op);
    cmd.append(' ');
    cmd.append(param1);
    cmd.append(' ');
    cmd.append(param2);
    writeToScript(cmd);
}

void MainWindow::numberControlValueChanged(double value)
{
    QWidget* widget = qobject_cast<QWidget*> (sender());
    if (!widget) return;
    sendControlCommand(QByteArrayLiteral("numbervalue"), widget->objectName().toUtf8(), QByteArray::number(value));
}

void MainWindow::hideHelpContents()
{
    // Hide all widgets in help dock
    for (int i=0; i<ui->helpLayout->count(); i++) {
        QWidget* widget = ui->helpLayout->itemAt(i)->widget();
        if (widget != ui->helpSearchBox) widget->hide();
    }
}

#include <QStringListModel>

class HelpModel : public QStringListModel
{
public:
    explicit HelpModel(QObject* parent = 0) : QStringListModel(parent) {}

    QVariant data(const QModelIndex &index, int role) const {
        if (role == Qt::EditRole) {
            QString str = QStringListModel::data(index, Qt::EditRole).toString();
            return str.mid(str.lastIndexOf('.')+1);
        }
        return QStringListModel::data(index, role);
    }
};

void MainWindow::loadHelpData()
{
    // Load json help
    QString path = QString("%1/lib/help.json").arg(nexpoPath());
    QFileInfo info(path);
    if (!info.exists()) {
        std::cerr << "Help file not found at " << path.toUtf8().constData() << std::endl;
        return;
    }
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Help not loaded: failed to open " << path.toUtf8().constData() << std::endl;
        return;
    }
    QByteArray json = file.readAll();
    file.close();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (!doc.isObject()) {
        std::cerr << "Error parsing help file: " << err.errorString().toUtf8().constData() << std::endl;
        return;
    }
    m_helpData = new QJsonObject(doc.object());

    // Create completer for help search box and APIs for editor
    m_helpModel = new HelpModel(this);

    for (auto p = m_helpData->constBegin(); p != m_helpData->constEnd(); p++) {
        // Get help name and shortname (eg nexpo.graphics.draw and draw)
        QJsonObject obj = p.value().toObject();
        QString name = obj.value("name").toString();
        if (name.size() == 0) continue;
        QString shortname = name.mid(name.lastIndexOf('.')+1);

        QModelIndex index = m_helpModel->index(m_helpModel->rowCount()-1);
        m_helpModel->insertRow(m_helpModel->rowCount());
        m_helpModel->setData(index, name, Qt::DisplayRole);

        // Add name(param1, param2) to api entries
        QStringList paramlist;
        QJsonArray params = obj.value("pnames").toArray();
        for (int i=0; i<params.size(); i++) {
            QString param = params[i].toString();
            if (param.size()) paramlist.append(param);
        }
        QString entry = shortname + "(" + paramlist.join(", ") + ")";
        FileEditor::addApiEntry(entry);
    }

    // Set completers
    QCompleter* helpCompleter = new QCompleter(m_helpModel, this);
    helpCompleter->setFilterMode(Qt::MatchContains);
    helpCompleter->setCompletionRole(Qt::DisplayRole);
    ui->helpSearchBox->setCompleter(helpCompleter);
    QCompleter* consoleCompleter = new QCompleter(m_helpModel, this);
    consoleCompleter->setCompletionRole(Qt::DisplayRole);
    consoleCompleter->setFilterMode(Qt::MatchContains);
    ui->console->setCompleter(consoleCompleter);

    // Prepare apis
    FileEditor::prepareApi();


}

bool MainWindow::showHelp(const QString& str)
{
    if (m_helpData == 0 || m_helpModel == 0) return false;

    // Show help dock
    ui->helpDockWidget->show();
    ui->helpSearchBox->setText(str);
    hideHelpContents();

    // Look up full name
    QString target = str.trimmed().toLower();
    if (target.size() == 0) return false;

    // Try prefix match on full names
    QModelIndexList matches = m_helpModel->match(m_helpModel->index(0), Qt::DisplayRole, target, Qt::MatchStartsWith | Qt::MatchFixedString);
    if (matches.count() == 0) {
        // Then try prefix match on short names
        matches = m_helpModel->match(m_helpModel->index(0), Qt::EditRole, target, Qt::MatchStartsWith | Qt::MatchFixedString);
    }

    // Always use full name for lookup
    if (matches.count() > 0) {
        target = m_helpModel->data(matches[0], Qt::DisplayRole).toString();
    }

    // Get help table for this string

    QJsonValue helpValue = m_helpData->value(target);
    if (!helpValue.isObject()) {
        ui->helpDescription->setText("No help available for “" + str + "”");
        ui->helpDescription->show();
        return false;
    }
    QJsonObject help = helpValue.toObject();

    // Summary
    QString summary = help.value("summary").toString();
    if (summary.size() > 0) {
        ui->helpSummary->setText(summary);
        ui->helpSummary->show();
    } else {
        ui->helpSummary->hide();
    }

    // Parameters
    QJsonArray pnames = help.value("pnames").toArray();
    QJsonArray pdesc = help.value("pdesc").toArray();
    QStringList paramlist;
    // Clear old params
    while (ui->helpParametersLayout->count() > 0) {
        delete ui->helpParametersLayout->itemAt(0)->widget();
    }
    int nparam = qMin(pnames.size(), pdesc.size());
    if (nparam > 0) {
        for (int i=0; i<nparam; i++) {
            QLabel* desc = new QLabel;
            desc->setText(pdesc[i].toString());
            desc->setWordWrap(true);
            ui->helpParametersLayout->addRow(pnames[i].toString(), desc);
            ((QLabel*)(ui->helpParametersLayout->labelForField(desc)))->setAlignment(Qt::AlignTop);
            paramlist += pnames[i].toString();
        }
        ui->helpParametersContainer->show();
    } else {
        ui->helpParametersContainer->hide();
    }

    // Name
    QString name = help.value("name").toString();
    ui->helpTitle->setText(name + " (" + paramlist.join(", ") + ")");
    ui->helpTitle->setVisible(name.size() > 0);

    // Description
    QString desc = help.value("description").toString();
    if (desc.startsWith((summary))) {
        desc = desc.mid(summary.size()).trimmed();
    }
    if (desc.size() > 0 && desc != summary) {
        ui->helpDescription->setText(desc);
        ui->helpDescription->show();
    } else {
        ui->helpDescription->hide();
    }

    // Example
    QString example = help.value("usage").toString();
    if (example.size() > 0) {
        ui->helpExample->setText(example);
        ui->helpExampleRow->show();
    } else {
        ui->helpExampleRow->hide();
    }

    // See
    // Clear old buttons
    while (ui->helpSeeLayout->count() > 0) {
        delete ui->helpSeeLayout->itemAt(0)->widget();
    }
    QJsonArray see = help.value("see").toArray();
    if (see.count() > 0) {
        // Add new buttons
        for (int i=0; i<see.count(); i++) {
            QString seeTarget = see[i].toString();
            if (seeTarget.size() > 0) {
                LinkButton* btn = new LinkButton;
                btn->setCursor(Qt::PointingHandCursor);
                QAction* action = new QAction(btn);
                action->setText(seeTarget);
                action->setData(seeTarget);
                action->setToolTip("Show help for “" + seeTarget + "”");
                connect(action, &QAction::triggered, this, &MainWindow::helpActionTriggered);
                btn->setDefaultAction(action);
                ui->helpSeeLayout->addWidget(btn);
            }
        }
    }
    ui->helpSeeRow->setVisible(ui->helpSeeLayout->count() > 0);

    return true;
}

void MainWindow::helpActionTriggered()
{
    QAction* action = qobject_cast<QAction*> (sender());
    if (action) {
        showHelp(action->data().toString());
    }
}

void MainWindow::deleteControl(const QString& name, int type) {
    for (int i=0; i<ui->controlsLayout->rowCount(); i++) {
        QLayoutItem* item = ui->controlsLayout->itemAt(i, QFormLayout::FieldRole);
        if (item) {
            Control* oldSlider = qobject_cast<Control*>(item->widget());
            if (oldSlider && oldSlider->type() == type && oldSlider->objectName() == name) {
                delete item->widget();
                QLayoutItem* labelItem = ui->controlsLayout->itemAt(i, QFormLayout::LabelRole);
                if (labelItem) delete labelItem->widget();
            }
        }
    }
}

void MainWindow::controls_createSlider(const QByteArray& name, const QByteArray& minValueString,
                                              const QByteArray& maxValueString, const QByteArray& initialValueString)
{
    // Delete old slider if it exists
    deleteControl(name, Control::Slider);

    bool ok;
    double minValue = minValueString.toDouble(&ok);
    if (!ok) return;
    double maxValue = maxValueString.toDouble(&ok);
    if (!ok) return;
    double initialValue = initialValueString.toDouble(&ok);
    if (!ok) return;

    SliderWithSpinner* widget = new SliderWithSpinner;
    widget->setObjectName(name);
    widget->setMinValue(minValue);
    widget->setMaxValue(maxValue);
    widget->setValue(initialValue);
    connect(widget, &SliderWithSpinner::valueChanged, this, &MainWindow::numberControlValueChanged);
    QLabel* label = new QLabel;
    label->setText(name);
    ui->controlsLayout->addRow(label, widget);
    ui->controlsDockWidget->show();
}

void MainWindow::controls_setNumberValue(const QByteArray& name, const QByteArray& valueString)
{
    bool ok;
    double value = valueString.toDouble(&ok);
    if (!ok) return;

    // Note: there could be multiple controls with the same name
    for (int row=0; row<ui->controlsLayout->rowCount(); row++) {
        QLayoutItem* item = ui->controlsLayout->itemAt(row, QFormLayout::FieldRole);
        if (!item) continue;
        Control* control = qobject_cast<Control*> (item->widget());

        if (control->objectName() == name) {
            switch (control->type())
            {
            case Control::Slider:
                (qobject_cast<SliderWithSpinner*> (control))->setValue(value);
                break;
            case Control::TimeSeriesPlot:
                (qobject_cast<TimeSeriesPlot*> (control))->addValue(m_scriptElapsed, value);
                break;
            default:
                break;
            }
        }
    }
}

void MainWindow::controls_createTimeSeriesPlot(const QByteArray &name)
{
    TimeSeriesPlot* plot = new TimeSeriesPlot;
    plot->setObjectName(name);
    plot->setTitle(name);
    plot->setWindowSize(200);
    //plot->setMinimumPlotInterval(1.0/29);
    //ui->controlsLayout->setWidget(ui->controlsLayout->rowCount(), QFormLayout::SpanningRole, plot);
    connect(this, &MainWindow::scriptElapsedChanged, plot, &TimeSeriesPlot::replot);
    ui->controlsLayout->addRow(plot);
    ui->controlsDockWidget->show();
}

void MainWindow::controls_setStringValue(const QByteArray &, const QByteArray &)
{

}

void MainWindow::controls_createCheckbox(const QByteArray& name, const QByteArray& checked)
{

}

void MainWindow::scriptProcessStderrReady() {
    ui->console->printError(m_scriptProcess->readAllStandardError());
}

void MainWindow::scriptProcessStdoutReady() {

    while (true) {
        QByteArray line = m_scriptProcess->readLine();
        if (line.endsWith('\n')) line.chop(1);
        if (line.size() == 0) break;
        if (line.startsWith(kControlPrefix)) {
            handleControlCommand(line.mid(1));
        } else {
            ui->console->print(line);
        }
    }
}

FileEditor* MainWindow::currentEditor() const {
    return m_currentEditor;
}

void MainWindow::updateCurrentEditor()
{
    FileEditor* editor = qobject_cast<FileEditor*>(focusWidget());
    if (editor != 0 && editor != m_currentEditor) {
        m_currentEditor = editor;
        emit currentEditorChanged(m_currentEditor);
    }
}

void MainWindow::scriptProcessStateChanged(QProcess::ProcessState state)
{
    switch(state) {
    case QProcess::NotRunning:
    {
        m_scriptProcess = 0;
        m_scriptRunning = false;
        emit scriptRunningChanged(m_scriptRunning);
        qint64 msecs = QDateTime::currentMSecsSinceEpoch() - m_scriptStartTime;
        QFileInfo info(m_runningScriptPath);
        ui->console->print(QString("%1 stopped after %2 seconds")
                           .arg(info.fileName())
                           .arg(msecs/1000.0));

        if (ui->actionAuto_hide_console->isChecked()) {
            ui->consoleDockWidget->hide();
        }
        clearControls();
        setStopAndRun();
        startScriptProcess();
        break;
    }
    case QProcess::Starting:
        setStopAndRun();
        break;
    case QProcess::Running:
        setStopAndRun();
        break;
    }
}

void MainWindow::runScript(const QString& scriptPath)
{
    if (scriptPath.size() == 0) return;

    // Check script exists
    QFileInfo scriptInfo(scriptPath);
    if (!scriptInfo.exists()) {
        std::cerr << "Error: file not found " << scriptPath.toUtf8().constData() << std::endl;
    }

    if (m_scriptProcess) {
        m_runningScriptPath = scriptInfo.absoluteFilePath();
        ui->console->print("Running " + scriptInfo.fileName());
        ui->actionConsole->setChecked(true);
        m_scriptProcess->write(m_runningScriptPath.toUtf8() + "\n");
        m_scriptRunning = true;
        m_scriptStartTime = QDateTime::currentMSecsSinceEpoch();
        m_scriptStatusWidget->setName(scriptInfo.fileName());
        m_scriptElapsed = 0;
        emit scriptElapsedChanged(m_scriptElapsed);
        emit scriptRunningChanged(m_scriptRunning);
        setStopAndRun();
    }
}

void MainWindow::stopCurrentScript() {
    if (m_scriptProcess) {
        m_scriptProcess->terminate();
        m_scriptRunning = false;
        emit scriptRunningChanged(m_scriptRunning);
    }
}

void MainWindow::setStopAndRun() {
    bool canRun = false, canStop = false;

    if (m_scriptProcess != 0) {
        if (m_scriptRunning) {
            canStop = true;
        } else {
            if (m_scriptProcess->state() == QProcess::Running) {
                // We can run if there is a valid editor
                if (currentEditor()) canRun = true;
            }
        }
    }


    ui->actionRun->setEnabled(canRun);
    ui->actionRun->setVisible(canRun);
    ui->actionStop->setEnabled(canStop);
    ui->actionStop->setVisible(canStop);
    // If can't do either, just show a disabled run button
    if (canRun == false && canStop == false) {
        ui->actionRun->setVisible(true);
    }

    if (m_scriptRunning) {
        QFileInfo info(m_runningScriptPath);
        ui->consoleDockWidget->setWindowTitle("Console - " + info.fileName());
    } else {
        ui->consoleDockWidget->setWindowTitle("Console");
    }
}

void MainWindow::on_actionRun_triggered()
{
    if (currentEditor() == 0) return;
    if (currentEditor()->path().size() == 0) {
        ui->actionSave_As->trigger();
    }
    if (currentEditor()->isModified()) {
        if (!currentEditor()->saveFile()) return;
    }
    runScript(currentEditor()->path());
}


void MainWindow::on_action_Preferences_triggered()
{
    loadFile(nexpoPath() + "/settings.lua");
}


void MainWindow::onCurrentEditorChanged(FileEditor* editor)
{
    setStopAndRun();    // we may be able to run now if we just opened a tab

    // If all editors closed
    if (ui->openTabs->count() == 0) {
        ui->findWidget->hide();
        ui->actionSave_As->setDisabled(true);
        ui->action_Save->setDisabled(true);
    }

    updateStatusBar();

    // close file action
    ui->actionClose_File->setEnabled(ui->openTabs->count() > 0);
    ui->actionClose_All_Files->setEnabled(ui->openTabs->count() > 0);

    if (editor)  {
        // window title and path
        setWindowFilePath(editor->path());
        setWindowTitle(editor->title() + " - " + qApp->applicationDisplayName());

        // save/save as actions
        ui->actionSave_As->setEnabled(true);
        ui->action_Save->setEnabled(true);

        // copy/paste actions
        ui->actionCopy->setEnabled(editor->hasSelectedText());
        ui->actionCut->setEnabled(editor->hasSelectedText());

        // re-run find if find widget is visible
        if (ui->findWidget->isVisible()) {
            editor->highlightFindText(ui->findLineEdit->text());
        }
    } else {
        setWindowTitle(qApp->applicationDisplayName());
        setWindowFilePath(QString());
    }
}


void MainWindow::showCentralWidget()
{
    if (ui->openTabs->count() == 0) {
        ui->centralStackedWidget->setCurrentWidget(ui->welcomeWidget);
        ui->closeWelcomeButton->hide();
        ui->consoleDockWidget->hide();
    } else {
        ui->centralStackedWidget->setCurrentWidget(ui->editorContainerWidget);
    }
}

void MainWindow::setupWelcomeWidget()
{
    QSettings settings;

    // Check for update
    ui->updateAvailableWidget->hide();
    if (settings.value("checkForUpdates", true).toBool()) {
        m_updateChecker = new UpdateChecker(this);
        connect(m_updateChecker, &UpdateChecker::gotLatestVersion, this, &MainWindow::showLatestVersion);
        m_updateChecker->checkForUpdate();
    }

    // Connect buttons
    connect(ui->newFileButton, &QAbstractButton::clicked, ui->actionNew, &QAction::trigger);
    connect(ui->openFileButton, &QAbstractButton::clicked, ui->action_Open, &QAction::trigger);

    // Populate examples list
    QDir exdir(nexpoPath() + "/examples");
    if (exdir.exists()) {
        QFileIconProvider ip;
        QFileInfoList files = exdir.entryInfoList(QDir::Files, QDir::Name | QDir::IgnoreCase);
        for (int i=0; i<files.count(); i++) {
            QFileInfo info = files[i];
            if (info.suffix().compare("nexpo", Qt::CaseInsensitive) == 0) {
                LinkButton* btn = new LinkButton(ui->examplesContainer);
                QAction* action = new QAction(btn);
                action->setData(info.absoluteFilePath());
                action->setText(info.fileName());
                action->setToolTip("Open " + info.fileName());
                connect(action, &QAction::triggered, this, &MainWindow::openFileAction);
                ui->menuOpen_Example->addAction(action);
                btn->setDefaultAction(action);
                btn->setIcon(ip.icon(info));
                btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                btn->setCursor(Qt::PointingHandCursor);
                ui->examplesContainer->layout()->addWidget(btn);
            }
        }
    }

    // Create "restore last session" button
    m_lastSessionFiles = settings.value("lastSession").toStringList();
    m_lastSessionCurrentFile = settings.value("lastSessionCurrentFile").toString();
    if (m_lastSessionFiles.count() > 0) {
        QDateTime lastSessionTime = settings.value("lastSessionTime").toDateTime();
        QString tooltip;

        // Start tooltip with timestamp if we have it
        if (lastSessionTime.isValid()) {
            tooltip = lastSessionTime.toString("MMM d, h:mm ap: ");
        }

        // Append filenames to tooltip
        QStringList filenames;
        for (int i=0; i<m_lastSessionFiles.count(); i++) {
            QFileInfo info(m_lastSessionFiles[i]);
            filenames.append(info.fileName());
        }
        tooltip += filenames.join(", ");

        ui->actionRestore_Last_Session->setToolTip(tooltip);
        ui->actionRestore_Last_Session->setEnabled(true);
        LinkButton* restoreButton = new LinkButton;
        restoreButton->setObjectName("restoreButton");
        ui->actionRestore_Last_Session->setText(QString("Restore last session (%1 file%2)")
                                                .arg(m_lastSessionFiles.count())
                                                .arg(m_lastSessionFiles.count() == 1 ? "" : "s"));

        restoreButton->setCursor(Qt::PointingHandCursor);
        restoreButton->setDefaultAction(ui->actionRestore_Last_Session);
        QWidget* spacer = new QWidget(this);
        spacer->setFixedHeight(15);
        ui->recentFilesContainer->layout()->addWidget(spacer);
        ui->recentFilesContainer->layout()->addWidget(restoreButton);
    }
}

void MainWindow::on_actionStop_triggered()
{
    stopCurrentScript();
}

void MainWindow::on_actionFind_triggered()
{
    if (ui->openTabs->count() == 0) return;

    ui->findWidget->show();
    ui->gotoLineWidget->hide();
    ui->findLineEdit->setFocus();
    ui->findLineEdit->selectAll();
}

void MainWindow::on_consoleDockWidget_visibilityChanged(bool visible)
{
    if (visible) ui->console->setFocus();
}

void MainWindow::on_findLineEdit_textChanged(const QString& text)
{
    if (currentEditor()) currentEditor()->highlightFindText(text);
}

void MainWindow::on_actionClose_File_triggered()
{
    closeEditor(currentEditor());
}

void MainWindow::on_actionFind_next_triggered()
{
    if (currentEditor()) currentEditor()->findNextForward();
}

void MainWindow::on_actionFind_Previous_triggered()
{
    if (currentEditor()) currentEditor()->findNextBackward();
}

void MainWindow::on_actionRedo_triggered()
{
    if (currentEditor()) currentEditor()->redo();
}

void MainWindow::on_actionUndo_triggered()
{
    if (currentEditor()) currentEditor()->undo();
}

void MainWindow::on_actionCut_triggered()
{
    if (currentEditor()) currentEditor()->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    if (currentEditor()) currentEditor()->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    if (currentEditor()) currentEditor()->paste();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutForm* about = new AboutForm(this);
    about->setAttribute(Qt::WA_DeleteOnClose, true);
    about->show();
}

void MainWindow::on_actionClose_All_Files_triggered()
{
    closeAllTabs();
}


void MainWindow::on_actionWelcome_triggered()
{
    ui->centralStackedWidget->setCurrentWidget(ui->welcomeWidget);
    ui->closeWelcomeButton->setVisible(ui->openTabs->count() > 0);
    ui->closeWelcomeButton->setCursor(Qt::PointingHandCursor);
}

void MainWindow::on_closeWelcomeButton_clicked()
{
    showCentralWidget();
}

void MainWindow::on_openTabs_currentChanged(int)
{
    // May need to show welcome screen if all tabs closed
    showCentralWidget();

    // Give editor focus (doesn't happen by default)
    // If we don't do this then currentEditor may not change when switching tabs,
    // until the user clicks inside the editor
    QWidget* widget = ui->openTabs->currentWidget();
    if (widget) widget->setFocus();
}

void MainWindow::on_actionRestore_Last_Session_triggered()
{
    for (int i=0; i<m_lastSessionFiles.count(); i++) {
        loadFile(m_lastSessionFiles[i]);
    }
    loadFile(m_lastSessionCurrentFile);      // will make tab active
}

void MainWindow::on_gotoLineValue_editingFinished()
{
    if (ui->gotoLineValue->hasFocus()) {
        if (currentEditor()) {
            int line = ui->gotoLineValue->value();
            if (line < 1 || line > currentEditor()->lines()) {
                // invalid line, ignore but don't hide widget
                return;
            }
            // scroll to end first then scroll to line
            // that way the cursor is at the top of the screen
            currentEditor()->setCursorPosition(currentEditor()->lines(), 0);
            currentEditor()->setCursorPosition(line-1, 0);
            currentEditor()->ensureCursorVisible();

        }
    }
    ui->gotoLineWidget->hide();
}

void MainWindow::on_actionGo_To_Line_triggered()
{
    if (currentEditor()) {
        int line, index;
        currentEditor()->getCursorPosition(&line, &index);
        ui->gotoLineValue->setValue(line+1);
        ui->findWidget->hide();
        ui->gotoLineWidget->show();
        ui->gotoLineValue->setFocus();
        ui->gotoLineValue->selectAll();
    }
}

void MainWindow::on_actionHelp_triggered()
{
    if (currentEditor() && currentEditor()->hasFocus()) {
        // Help for current word in editor
        int line, index;
        currentEditor()->getCursorPosition(&line, &index);
        showHelp(currentEditor()->wordAtLineIndex(line, index));
    } else if (ui->console->hasFocus()) {
        // Help for current word in console
        showHelp(ui->console->currentWord());
        if (ui->console->currentWord().size() == 0 && ui->helpDockWidget->isVisible()) {
            ui->helpSearchBox->setFocus();
        }
    } else {
        showHelp(QString());
        if (ui->helpDockWidget->isVisible()) {
            ui->helpSearchBox->setFocus();
            ui->helpSearchBox->selectAll();
        }
    }
}

void MainWindow::on_helpSearchBox_returnPressed()
{
    if (showHelp(ui->helpSearchBox->text())) {
        ui->helpSearchBox->selectAll();
    }
}

void MainWindow::on_actionClear_Recent_Files_triggered()
{
    QSettings settings;
    settings.remove("recentFileList");
    settings.remove("lastSession");
    settings.remove("lastSessionCurrentFile");
    delete findChild<LinkButton*>("restoreButton");
    updateRecentFileActions();
}
