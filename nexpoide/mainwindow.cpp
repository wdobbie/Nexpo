#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileeditor.h"
#include "outputredirector.h"
#include "updatechecker.h"
#include "aboutform.h"
#include "application.h"

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
#include <iostream>

#include "scriptstatuswidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scriptProcess(0)
    , m_updateChecker(0)
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

    // Create script status widget
    ScriptStatusWidget* scriptStatusWidget = new ScriptStatusWidget(this);
    connect(this, &MainWindow::scriptElapsedChanged, scriptStatusWidget, &ScriptStatusWidget::setElapsedSeconds);
    connect(this, &MainWindow::scriptFilenameChanged, scriptStatusWidget, &ScriptStatusWidget::setName);

    // Add left spacer
    QWidget* spacer1 = new QWidget(this);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionConsole, spacer1);

    // Add status widget
    QAction* scriptStatusAction = ui->mainToolBar->insertWidget(ui->actionConsole, scriptStatusWidget);
    scriptStatusAction->setVisible(false);
    connect(this, &MainWindow::scriptRunningChanged, scriptStatusAction, &QAction::setVisible);

    // Add right spacer
    QWidget* spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionConsole, spacer2);

    // Restore window state
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

    // Hide find widget (can't do this in form designer)
    ui->findWidget->hide();

    // Populate recent files menu
    createRecentFileActions();

    // Welcome widget
    setupWelcomeWidget();

    // Set initial state of central widget
    showCentralWidget();

    // Create process to run scripts
    startScriptProcess();

    // Connect console input commands to script stdinput
    connect(ui->console, &Console::command, this, &MainWindow::sendScriptCommand);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Returns true if all tabs close, false if user cancelled
bool MainWindow::closeAllTabs()
{
    while (ui->openTabs->count() > 0) {
        if (!closeTab(ui->openTabs->count()-1)) {
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event) {
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

    if (!closeAllTabs()) {
        event->ignore();
        return;
    }

    // It's either stopped or we're pretending it is
    m_scriptRunning = false;

    // Save window arrangement
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
}

static QString strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::addEditor(FileEditor* editor, const QString& name)
{
    // The status bar connection must be queued, or else there is a crash during what looks like scintilla's paint event
    connect(editor, &QsciScintilla::cursorPositionChanged, this, &MainWindow::updateStatusBar, Qt::QueuedConnection);
    connect(editor, &QsciScintilla::modificationChanged, this, &MainWindow::updateTabTitles);
    connect(editor, &FileEditor::titleChanged, this, &MainWindow::updateTabTitles);
    connect(editor, &FileEditor::escapePressed, [=]() {
       if (ui->consoleDockWidget->isVisible()) {
           ui->consoleDockWidget->hide();
       } else if (ui->findWidget->isVisible()) {
           ui->findWidget->hide();
       }
    });
    // clear highlighted text when find box closes
    connect(ui->findLineEdit, &FindLineEdit::hidden, editor, &FileEditor::unhighlightFindText);

    // status message
    connect(editor, &FileEditor::statusMessageChanged, this, &MainWindow::editorStatusMessageChanged, Qt::QueuedConnection);

    connect(editor, &FileEditor::copyAvailable, [=](bool avail){
       ui->actionCopy->setEnabled(avail);
       ui->actionCut->setEnabled(avail);
    });

    int index = ui->openTabs->addTab(editor, name);
    ui->openTabs->setCurrentIndex(index);
}

void MainWindow::loadFile(const QString& path)
{
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
        std::cerr << "Failed to open " << path.toUtf8().data() << std::endl;
        delete editor;
        return;
    }

    addEditor(editor, strippedName(path));
    addRecentFile(editor->path());
    showCentralWidget();
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
    FileEditor* editor = new FileEditor(this);
    addEditor(editor, "Untitled");
    editor->beginUndoAction();
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
    editor->endUndoAction();
    editor->setModified(false);
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

void MainWindow::sendScriptCommand(const QString& cmd)
{
    if (m_scriptProcess && m_scriptRunning) {
        m_scriptProcess->write(cmd.toUtf8());
        m_scriptProcess->write("\n");
    } else {
        ui->console->printError("No script is running\n");
    }
}

void MainWindow::updateStatusBar()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor == 0) {
        ui->statusBar->clearMessage();
    } else {
        if (!ui->findWidget->isVisible() || ui->findLineEdit->text().size() == 0) {
            int line, index;
            editor->getCursorPosition(&line, &index);
            ui->statusBar->showMessage(QString("Line %1, Column %2").arg(line+1).arg(index+1));
        }
    }
}

void MainWindow::editorStatusMessageChanged(const QString &msg, int timeout)
{
    if (ui->openTabs->currentWidget() == sender()) {
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

void MainWindow::showLatestVersion(const QString &version, const QString &link)
{
    if (isNewerVersion(version)) {
        ui->updateVersion->setText(version);
        ui->updateUrl->setText(link);
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
    updateRecentFileActions();
}

// Returns false if user cancelled the action, true otherwise
bool MainWindow::closeTab(int index)
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->widget(index));
    if (!editor) return true;

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

    ui->openTabs->removeTab(index);
    delete editor;

    return true;
}

void MainWindow::on_openTabs_tabCloseRequested(int index)
{
    closeTab(index);
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

    ui->recentFilesListWidget->clear();

    QFileIconProvider ip;
    for (int i=0; i<numRecentFiles; i++) {
        recentFileActions[i]->setText(files[i]);
        recentFileActions[i]->setData(files[i]);
        recentFileActions[i]->setVisible(true);

        QFileInfo info(files[i]);

        QListWidgetItem* item = new QListWidgetItem(ui->recentFilesListWidget);
        ui->recentFilesListWidget->addItem(item);
        QToolButton* btn = new QToolButton(ui->recentFilesListWidget);
        btn->setDefaultAction(recentFileActions[i]);
        if (info.exists()) {
            btn->setIcon(ip.icon(info));
        } else {
            btn->setIcon(QProxyStyle().standardIcon(QStyle::SP_FileIcon));
        }
        btn->setStyleSheet("QToolButton { background: transparent; border: none; text-decoration: underline; color: blue; font-size: 12pt;}");
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setCursor(Qt::PointingHandCursor);
        ui->recentFilesListWidget->setItemWidget(item, btn);
    }

    for (int i=numRecentFiles; i<MaxRecentFiles; i++) {
        recentFileActions[i]->setVisible(false);
    }
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
}

// TODO: should be tristate: saved, not saved, cancelled?
// So that the cancelled signal can be bubbled up to closeEvent
bool MainWindow::editorSaveAs(FileEditor* editor)
{
    if (editor == 0) return false;

    QString savePath = editor->path();
    QSettings settings;
    if (savePath.size() == 0) {
        savePath = settings.value("lastSavePath",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString()
                + "/" + editor->title();
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Save As", savePath, "Nexpo files (*.nexpo *.lua);;All files (*.*)");

    if (fileName.size() == 0) return false;

    QFileInfo info(fileName);
    if (info.exists()) {
        settings.setValue("lastSavePath", info.absolutePath());
    }

    if (!editor->saveFileAs(fileName)) {
        std::cerr << "Failed to save " << info.absoluteFilePath().toUtf8().data() << std::endl;
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
        std::cerr << "Failed to save " << editor->path().toUtf8().data() << std::endl;
        return false;
    } else {
        addRecentFile(editor->path());
        return true;
    }
}

void MainWindow::on_action_Save_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor == 0) return;

    editorSave(editor);
}

void MainWindow::on_actionSave_As_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    editorSaveAs(editor);
}


QString MainWindow::nexpoPath() {
    QSettings settings;
    return settings.value("nexpoPath", appRootPath()).toString();
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

void MainWindow::scriptProcessStderrReady() {
    ui->console->printError(m_scriptProcess->readAllStandardError());
}

void MainWindow::scriptProcessStdoutReady() {
    ui->console->print(m_scriptProcess->readAllStandardOutput());
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
        std::cerr << "Error: file not found " << scriptPath.toUtf8().data() << std::endl;
    }

    if (m_scriptProcess) {
        m_runningScriptPath = scriptInfo.absoluteFilePath();
        ui->console->print("Running " + scriptInfo.fileName());
        ui->actionConsole->setChecked(true);
        m_scriptProcess->write(m_runningScriptPath.toUtf8() + "\n");
        m_scriptRunning = true;
        emit scriptRunningChanged(m_scriptRunning);
        emit scriptFilenameChanged(scriptInfo.fileName());
        emit scriptElapsedChanged(0);
        m_scriptStartTime = QDateTime::currentMSecsSinceEpoch();
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

                FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
                if (editor != 0) canRun = true;
            }
        }
    }

    ui->actionRun->setEnabled(canRun);
    ui->actionRun->setVisible(canRun);
    ui->actionStop->setEnabled(canStop);
    ui->actionStop->setVisible(canStop);

    if (m_scriptRunning) {
        QFileInfo info(m_runningScriptPath);
        ui->consoleDockWidget->setWindowTitle("Console - " + info.fileName());
    } else {
        ui->consoleDockWidget->setWindowTitle("Console");
    }
}

void MainWindow::on_actionRun_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor == 0) return;
    if (editor->path().size() == 0) {
        ui->actionSave_As->trigger();
    }
    if (editor->isModified()) {
        if (!editor->saveFile()) return;
    }
    runScript(editor->path());
}


void MainWindow::on_action_Preferences_triggered()
{
    loadFile(nexpoPath() + "/settings.lua");
}


void MainWindow::on_openTabs_currentChanged(int)
{
    setStopAndRun();    // we may be able to run now if we just opened a tab
    if (ui->openTabs->count() == 0) {
        ui->findWidget->hide();
        ui->actionSave_As->setDisabled(true);
        ui->action_Save->setDisabled(true);
    }
    updateStatusBar();
    showCentralWidget();

    // close file
    ui->actionClose_File->setEnabled(ui->openTabs->count() > 0);
    ui->actionClose_All_Files->setEnabled(ui->openTabs->count() > 0);

    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (!editor) return;

    // save/save as
    ui->actionSave_As->setDisabled(false);
    ui->action_Save->setDisabled(false);

    // copy/paste
    ui->actionCopy->setEnabled(editor->hasSelectedText());
    ui->actionCut->setEnabled(editor->hasSelectedText());

    // re-run find if find widget is visible
    if (ui->findWidget->isVisible()) {
        editor->highlightFindText(ui->findLineEdit->text());
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

    // Don't show blue focus rectangle on OS X
    ui->recentFilesListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->examplesListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

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
                QListWidgetItem* item = new QListWidgetItem(ui->examplesListWidget);
                ui->examplesListWidget->addItem(item);
                QToolButton* btn = new QToolButton(ui->examplesListWidget);
                QAction* action = new QAction(btn);
                action->setData(info.absoluteFilePath());
                action->setText(info.fileName());
                connect(action, &QAction::triggered, this, &MainWindow::openFileAction);
                ui->menuOpen_Example->addAction(action);
                btn->setDefaultAction(action);
                btn->setIcon(ip.icon(info));
                btn->setStyleSheet("QToolButton { background: transparent; border: none; text-decoration: underline; color: blue; font-size: 12pt;}");
                btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                btn->setCursor(Qt::PointingHandCursor);
                ui->examplesListWidget->setItemWidget(item, btn);
            }
        }
    }
}

void MainWindow::on_actionStop_triggered()
{
    stopCurrentScript();
}

void MainWindow::on_actionFind_triggered()
{
    if (ui->openTabs->count() == 0) return;

    ui->findWidget->setVisible(true);
    ui->findLineEdit->setFocus();
    ui->findLineEdit->selectAll();
}

void MainWindow::on_consoleDockWidget_visibilityChanged(bool visible)
{
    if (visible) ui->console->setFocus();
}

void MainWindow::on_findLineEdit_textChanged(const QString& text)
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (!editor) return;
    editor->highlightFindText(text);
}

void MainWindow::on_actionClose_File_triggered()
{
    int index = ui->openTabs->currentIndex();
    if (index >= 0) on_openTabs_tabCloseRequested(index);
}

void MainWindow::on_actionFind_next_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (!editor) return;
    editor->findNextForward();
}

void MainWindow::on_actionFind_Previous_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (!editor) return;
    editor->findNextBackward();
}

void MainWindow::on_actionRedo_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor) editor->redo();
}

void MainWindow::on_actionUndo_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor) editor->undo();
}

void MainWindow::on_actionCut_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor) editor->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor) editor->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    FileEditor* editor = qobject_cast<FileEditor*> (ui->openTabs->currentWidget());
    if (editor) editor->paste();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutForm* about = new AboutForm(this);
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
