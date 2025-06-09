#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QDir>
#include <QJsonObject>

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);

    bool openProject(const QString &projectPath);
    bool createProject(const QString &projectPath, const QString &projectName, const QString &projectType);
    void closeProject();
    
    QString currentProjectPath() const { return m_currentProjectPath; }
    QString currentProjectName() const { return m_currentProjectName; }
    QStringList projectFiles() const { return m_projectFiles; }
    QStringList recentProjects() const { return m_recentProjects; }
    
    void addRecentProject(const QString &projectPath);
    void removeRecentProject(const QString &projectPath);

signals:
    void projectOpened(const QString &projectPath);
    void projectClosed();
    void fileAdded(const QString &filePath);
    void fileRemoved(const QString &filePath);
    void fileChanged(const QString &filePath);

private slots:
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);

private:
    void scanProjectFiles();
    void loadRecentProjects();
    void saveRecentProjects();
    void loadProjectSettings();
    void saveProjectSettings();
    void createQtProjectFiles(const QString &projectPath, const QString &projectName);
    void createConsoleProjectFiles(const QString &projectPath, const QString &projectName);
    void createLibraryProjectFiles(const QString &projectPath, const QString &projectName);
    
    QString m_currentProjectPath;
    QString m_currentProjectName;
    QStringList m_projectFiles;
    QStringList m_recentProjects;
    QFileSystemWatcher *m_fileWatcher;
    QJsonObject m_projectSettings;
};

#endif // PROJECTMANAGER_H
