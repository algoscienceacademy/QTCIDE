#ifndef TERMINAL_H
#define TERMINAL_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QLabel>
#include <QDirIterator>
#include <QSettings>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>

class Terminal : public QWidget
{
    Q_OBJECT

public:
    explicit Terminal(QWidget *parent = nullptr);
    
    void clear();
    void appendText(const QString &text);
    void setCurrentDirectory(const QString &path);
    QString currentDirectory() const { return m_currentDirectory; }
    void loadTerminalSettings();
    void applyTerminalSettings();
    QStringList detectAvailableTerminals();
    bool isTerminalAvailable(const QString &terminalType);
    QString getCurrentShellType() const { return m_shellType; }
    QString getShellExecutable() const;

signals:
    void directoryChanged(const QString &path);
    void fileSystemChanged();

private slots:
    void executeCommand();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessOutput();
    void onProcessError();

private:
    void setupUI();
    void applyTerminalStyle();
    void updatePrompt();
    void autoDetectTerminal();
    QString getTerminalExecutablePath(const QString &terminalType) const;
    void initializeTerminal();
    void switchToTerminal(const QString &terminalType);
    QString getTerminalDisplayName() const;
    void showHelp();
    void changeDirectory(const QString &path);
    void createDirectory(const QString &name);
    void createFile(const QString &fileName);
    void deleteFile(const QString &fileName);
    void deleteDirectory(const QString &dirName);
    void listDirectory();
    void showFileContent(const QString &fileName);
    void executeGitCommand(const QString &command);
    void executeBuildCommand(const QString &command);
    void executeSystemCommand(const QString &command);
    void showDirectoryTree();
    void showTree(const QString &path, int depth);
    void findFiles(const QString &pattern);
    
    QTextEdit *m_output;
    QLineEdit *m_input;
    QProcess *m_process;
    QLabel *m_promptLabel;
    
    QString m_currentDirectory;
    QString m_shellType;
    QString m_customShellPath;
    QStringList m_availableTerminals;
};

#endif // TERMINAL_H
