#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QProcess>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include <QClipboard>

class WelcomeScreen;
class Terminal;
class CodeEditor;
class ProjectManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void openFolder(); // Make this public so WelcomeScreen can access it
    void newProject(); // Make this public so WelcomeScreen can access it

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void build();
    void configure();
    void rebuild();
    void clean();
    void run();
    void runDebug();
    void showWelcome();
    void openFileFromPath(const QString &filePath);
    void openProject();
    void focusTerminal();
    void onProjectOpened(const QString &projectPath);
    void onProjectClosed();
    void onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onBuildOutput();
    void onBuildError();
    void onRunFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRunOutput();
    void onRunError();
    void showFileContextMenu(const QPoint &point);
    void showSettings();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void applyGlassmorphicStyle();
    
    QWidget *m_centralWidget;
    QStackedWidget *m_stackedWidget;
    WelcomeScreen *m_welcomeScreen;
    QSplitter *m_mainSplitter;
    QSplitter *m_rightSplitter;
    
    // Editor area
    CodeEditor *m_editor;
    
    // File explorer
    QTreeView *m_fileTree;
    QFileSystemModel *m_fileModel;
    
    // Terminal
    Terminal *m_terminal;
    
    // Build and run processes
    QProcess *m_buildProcess;
    QProcess *m_runProcess;
    
    // Project management
    ProjectManager *m_projectManager;
    
    QString m_currentProjectPath;
    QString m_currentFilePath;

    void saveFileToPath(const QString &filePath);
    void createNewFile(const QString &basePath);
    void createNewFolder(const QString &basePath);
    void renameFileOrFolder(const QModelIndex &index);
    void deleteFileOrFolder(const QModelIndex &index);
};

#endif // MAINWINDOW_H
