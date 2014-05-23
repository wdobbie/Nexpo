#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QByteArray>

namespace Ui {
class MainWindow;
}

class OutputRedirector;
class FileEditor;
class UpdateChecker;
class ScriptStatusWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadFile(const QString&);
    void runScript(const QString&);
    void stopCurrentScript();
    FileEditor* currentEditor() const;
    bool closeEditor(FileEditor*);

public slots:
    void openFileAction();
    void updateTabTitles();
    void writeToScript(const QByteArray&);
    void updateStatusBar();
    void editorStatusMessageChanged(const QString& msg, int timeout);
    void showLatestVersion(const QString& version, const QString& link, const QString& message);
    void scriptProcessStateChanged(QProcess::ProcessState state);
    void scriptProcessStderrReady();
    void scriptProcessStdoutReady();
    void updateCurrentEditor();
    void onCurrentEditorChanged(FileEditor*);
    void numberControlValueChanged(double value);

signals:
    void scriptRunningChanged(bool);
    void currentEditorChanged(FileEditor*);

private slots:
    void on_actionNew_triggered();

    void on_openTabs_tabCloseRequested(int index);

    void on_action_Open_triggered();

    void on_action_Save_triggered();

    void on_actionSave_As_triggered();

    void on_actionRun_triggered();

    void on_action_Preferences_triggered();

    void on_actionStop_triggered();

    void on_actionFind_triggered();

    void on_consoleDockWidget_visibilityChanged(bool visible);

    void on_findLineEdit_textChanged(const QString &arg1);

    void on_actionClose_File_triggered();

    void on_actionFind_next_triggered();

    void on_actionFind_Previous_triggered();

    void on_actionRedo_triggered();

    void on_actionUndo_triggered();

    void on_actionCut_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionAbout_triggered();

    void on_actionClose_All_Files_triggered();

    void on_actionWelcome_triggered();

    void on_closeWelcomeButton_clicked();

    void on_openTabs_currentChanged(int index);

    void on_actionRestore_Last_Session_triggered();

protected:
    virtual void closeEvent(QCloseEvent*);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);

private:
    Ui::MainWindow *ui;

    bool m_scriptRunning;

    enum { MaxRecentFiles = 30 };
    QAction* recentFileActions[MaxRecentFiles];

    void createRecentFileActions();
    void updateRecentFileActions();
    OutputRedirector* m_stdOutRedirector;
    OutputRedirector* m_stdErrRedirector;
    QProcess* m_scriptProcess;
    QString m_runningScriptPath;
    qint64 m_scriptStartTime;
    UpdateChecker* m_updateChecker;
    FileEditor* m_currentEditor;
    ScriptStatusWidget* m_scriptStatusWidget;

    void startScriptProcess();
    void addRecentFile(const QString& path);
    void setStopAndRun();
    void addEditor(FileEditor* editor);
    void showCentralWidget();
    void setupWelcomeWidget();
    QString nexpoPath();
    bool editorSave(FileEditor*);
    bool editorSaveAs(FileEditor*);
    bool closeTab(int);
    bool closeAllTabs();
    void handleControlCommand(QByteArray);
    int rowForControl(const QString&) const;
    void sendControlCommand(const QByteArray& op, const QByteArray& param1, const QByteArray& param2);
    void clearControls();

    void controls_createNumberControl(const QByteArray& name, const QByteArray& minValueString,
                                                  const QByteArray& maxValueString, const QByteArray& initialValueString);
    void controls_setNumberValue(const QByteArray&, const QByteArray&);
    void controls_setStringValue(const QByteArray&, const QByteArray&);
};

#endif // MAINWINDOW_H
