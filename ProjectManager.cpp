#include "ProjectManager.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QTextStream>
#include <QFile>

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
{
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &ProjectManager::onDirectoryChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ProjectManager::onFileChanged);
    
    loadRecentProjects();
}

bool ProjectManager::openProject(const QString &projectPath)
{
    if (!QDir(projectPath).exists()) {
        return false;
    }
    
    closeProject();
    
    m_currentProjectPath = projectPath;
    m_currentProjectName = QFileInfo(projectPath).baseName();
    
    // Add to file watcher
    m_fileWatcher->addPath(projectPath);
    
    // Scan for project files
    scanProjectFiles();
    
    // Load project settings
    loadProjectSettings();
    
    // Add to recent projects
    addRecentProject(projectPath);
    
    emit projectOpened(projectPath);
    return true;
}

bool ProjectManager::createProject(const QString &projectPath, const QString &projectName, const QString &projectType)
{
    QDir dir;
    if (!dir.mkpath(projectPath)) {
        return false;
    }
    
    m_currentProjectPath = projectPath;
    m_currentProjectName = projectName;
    
    // Create basic project structure
    dir.mkpath(projectPath + "/src");
    dir.mkpath(projectPath + "/include");
    dir.mkpath(projectPath + "/build");
    dir.mkpath(projectPath + "/docs");
    dir.mkpath(projectPath + "/tests");
    
    // Create CMakeLists.txt
    QFile cmakeFile(projectPath + "/CMakeLists.txt");
    if (cmakeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&cmakeFile);
        out << "cmake_minimum_required(VERSION 3.16)\n";
        out << "project(" << projectName << " VERSION 1.0.0 LANGUAGES CXX)\n\n";
        out << "set(CMAKE_CXX_STANDARD 17)\n";
        out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
        
        if (projectType == "Qt Application") {
            out << "find_package(Qt6 REQUIRED COMPONENTS Core Widgets)\n\n";
            out << "qt6_standard_project_setup()\n\n";
            out << "set(SOURCES\n";
            out << "    src/main.cpp\n";
            out << "    src/MainWindow.cpp\n";
            out << ")\n\n";
            out << "set(HEADERS\n";
            out << "    include/MainWindow.h\n";
            out << ")\n\n";
            out << "qt6_add_executable(" << projectName << " ${SOURCES})\n\n";
            out << "target_include_directories(" << projectName << " PRIVATE include)\n";
            out << "target_link_libraries(" << projectName << " PRIVATE Qt6::Core Qt6::Widgets)\n\n";
            out << "set_target_properties(" << projectName << " PROPERTIES\n";
            out << "    AUTOMOC ON\n";
            out << "    AUTOUIC ON\n";
            out << "    AUTORCC ON\n";
            out << ")\n";
        } else if (projectType == "Console Application") {
            out << "set(SOURCES\n";
            out << "    src/main.cpp\n";
            out << ")\n\n";
            out << "add_executable(" << projectName << " ${SOURCES})\n";
            out << "target_include_directories(" << projectName << " PRIVATE include)\n";
        } else if (projectType == "Static Library") {
            out << "set(SOURCES\n";
            out << "    src/library.cpp\n";
            out << ")\n\n";
            out << "set(HEADERS\n";
            out << "    include/library.h\n";
            out << ")\n\n";
            out << "add_library(" << projectName << " STATIC ${SOURCES})\n";
            out << "target_include_directories(" << projectName << " PUBLIC include)\n";
        }
    }
    
    // Create main files based on project type
    if (projectType == "Qt Application") {
        createQtProjectFiles(projectPath, projectName);
    } else if (projectType == "Console Application") {
        createConsoleProjectFiles(projectPath, projectName);
    } else if (projectType == "Static Library") {
        createLibraryProjectFiles(projectPath, projectName);
    }
    
    // Create README.md
    QFile readmeFile(projectPath + "/README.md");
    if (readmeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&readmeFile);
        out << "# " << projectName << "\n\n";
        out << "## Description\n\n";
        out << "A " << projectType.toLower() << " created with QTCIDE.\n\n";
        out << "## Build Instructions\n\n";
        out << "```bash\n";
        out << "mkdir build\n";
        out << "cd build\n";
        out << "cmake ..\n";
        out << "cmake --build .\n";
        out << "```\n";
    }
    
    // Create .gitignore
    QFile gitignoreFile(projectPath + "/.gitignore");
    if (gitignoreFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&gitignoreFile);
        out << "build/\n";
        out << "*.user\n";
        out << ".DS_Store\n";
        out << "Thumbs.db\n";
        out << "*.autosave\n";
        out << ".qtcide_project\n";
    }
    
    return openProject(projectPath);
}

void ProjectManager::createQtProjectFiles(const QString &projectPath, const QString &projectName)
{
    // Create main.cpp
    QFile mainFile(projectPath + "/src/main.cpp");
    if (mainFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&mainFile);
        out << "#include <QApplication>\n";
        out << "#include \"MainWindow.h\"\n\n";
        out << "int main(int argc, char *argv[])\n";
        out << "{\n";
        out << "    QApplication app(argc, argv);\n";
        out << "    app.setApplicationName(\"" << projectName << "\");\n\n";
        out << "    MainWindow window;\n";
        out << "    window.show();\n\n";
        out << "    return app.exec();\n";
        out << "}\n";
    }
    
    // Create MainWindow.h
    QFile headerFile(projectPath + "/include/MainWindow.h");
    if (headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&headerFile);
        out << "#ifndef MAINWINDOW_H\n";
        out << "#define MAINWINDOW_H\n\n";
        out << "#include <QMainWindow>\n\n";
        out << "class MainWindow : public QMainWindow\n";
        out << "{\n";
        out << "    Q_OBJECT\n\n";
        out << "public:\n";
        out << "    MainWindow(QWidget *parent = nullptr);\n";
        out << "    ~MainWindow();\n";
        out << "};\n\n";
        out << "#endif // MAINWINDOW_H\n";
    }
    
    // Create MainWindow.cpp
    QFile sourceFile(projectPath + "/src/MainWindow.cpp");
    if (sourceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&sourceFile);
        out << "#include \"MainWindow.h\"\n";
        out << "#include <QVBoxLayout>\n";
        out << "#include <QLabel>\n";
        out << "#include <QPushButton>\n\n";
        out << "MainWindow::MainWindow(QWidget *parent)\n";
        out << "    : QMainWindow(parent)\n";
        out << "{\n";
        out << "    auto *centralWidget = new QWidget;\n";
        out << "    setCentralWidget(centralWidget);\n\n";
        out << "    auto *layout = new QVBoxLayout(centralWidget);\n";
        out << "    auto *label = new QLabel(\"Welcome to " << projectName << "!\");\n";
        out << "    auto *button = new QPushButton(\"Click me!\");\n\n";
        out << "    layout->addWidget(label);\n";
        out << "    layout->addWidget(button);\n\n";
        out << "    setWindowTitle(\"" << projectName << "\");\n";
        out << "    resize(400, 300);\n";
        out << "}\n\n";
        out << "MainWindow::~MainWindow() = default;\n";
    }
}

void ProjectManager::createConsoleProjectFiles(const QString &projectPath, const QString &projectName)
{
    QFile mainFile(projectPath + "/src/main.cpp");
    if (mainFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&mainFile);
        out << "#include <iostream>\n";
        out << "#include <string>\n\n";
        out << "int main()\n";
        out << "{\n";
        out << "    std::cout << \"Welcome to " << projectName << "!\" << std::endl;\n";
        out << "    \n";
        out << "    std::string input;\n";
        out << "    std::cout << \"Enter your name: \";\n";
        out << "    std::getline(std::cin, input);\n";
        out << "    \n";
        out << "    std::cout << \"Hello, \" << input << \"!\" << std::endl;\n";
        out << "    \n";
        out << "    return 0;\n";
        out << "}\n";
    }
}

void ProjectManager::createLibraryProjectFiles(const QString &projectPath, const QString &projectName)
{
    // Create library.h
    QFile headerFile(projectPath + "/include/library.h");
    if (headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&headerFile);
        out << "#ifndef LIBRARY_H\n";
        out << "#define LIBRARY_H\n\n";
        out << "#include <string>\n\n";
        out << "namespace " << projectName.toLower() << " {\n\n";
        out << "class Library\n";
        out << "{\n";
        out << "public:\n";
        out << "    Library();\n";
        out << "    ~Library();\n\n";
        out << "    std::string getVersion() const;\n";
        out << "    void initialize();\n";
        out << "    void cleanup();\n";
        out << "};\n\n";
        out << "} // namespace " << projectName.toLower() << "\n\n";
        out << "#endif // LIBRARY_H\n";
    }
    
    // Create library.cpp
    QFile sourceFile(projectPath + "/src/library.cpp");
    if (sourceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&sourceFile);
        out << "#include \"library.h\"\n";
        out << "#include <iostream>\n\n";
        out << "namespace " << projectName.toLower() << " {\n\n";
        out << "Library::Library()\n";
        out << "{\n";
        out << "    std::cout << \"" << projectName << " library initialized\" << std::endl;\n";
        out << "}\n\n";
        out << "Library::~Library()\n";
        out << "{\n";
        out << "    std::cout << \"" << projectName << " library destroyed\" << std::endl;\n";
        out << "}\n\n";
        out << "std::string Library::getVersion() const\n";
        out << "{\n";
        out << "    return \"1.0.0\";\n";
        out << "}\n\n";
        out << "void Library::initialize()\n";
        out << "{\n";
        out << "    // Initialize library resources\n";
        out << "}\n\n";
        out << "void Library::cleanup()\n";
        out << "{\n";
        out << "    // Cleanup library resources\n";
        out << "}\n\n";
        out << "} // namespace " << projectName.toLower() << "\n";
    }
}

void ProjectManager::closeProject()
{
    if (!m_currentProjectPath.isEmpty()) {
        saveProjectSettings();
        m_fileWatcher->removePaths(m_fileWatcher->directories());
        m_fileWatcher->removePaths(m_fileWatcher->files());
        
        m_currentProjectPath.clear();
        m_currentProjectName.clear();
        m_projectFiles.clear();
        m_projectSettings = QJsonObject();
        
        emit projectClosed();
    }
}

void ProjectManager::scanProjectFiles()
{
    m_projectFiles.clear();
    
    if (m_currentProjectPath.isEmpty()) {
        return;
    }
    
    QDirIterator it(m_currentProjectPath, 
                    QStringList() << "*.cpp" << "*.h" << "*.hpp" << "*.c" << "*.cc" 
                                  << "*.cxx" << "*.cmake" << "CMakeLists.txt" << "*.pro"
                                  << "*.ui" << "*.qrc" << "*.qml" << "*.js",
                    QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        QString filePath = it.next();
        if (!filePath.contains("/build/") && !filePath.contains("/.git/")) {
            m_projectFiles << filePath;
            m_fileWatcher->addPath(filePath);
        }
    }
}

void ProjectManager::addRecentProject(const QString &projectPath)
{
    m_recentProjects.removeAll(projectPath);
    m_recentProjects.prepend(projectPath);
    
    // Keep only 10 recent projects
    while (m_recentProjects.size() > 10) {
        m_recentProjects.removeLast();
    }
    
    saveRecentProjects();
}

void ProjectManager::removeRecentProject(const QString &projectPath)
{
    m_recentProjects.removeAll(projectPath);
    saveRecentProjects();
}

void ProjectManager::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
    scanProjectFiles();
}

void ProjectManager::onFileChanged(const QString &path)
{
    emit fileChanged(path);
}

void ProjectManager::loadRecentProjects()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    QFile file(configPath + "/recent_projects.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray array = doc.array();
        
        for (const auto &value : array) {
            QString projectPath = value.toString();
            if (QDir(projectPath).exists()) {
                m_recentProjects << projectPath;
            }
        }
    }
}

void ProjectManager::saveRecentProjects()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    QFile file(configPath + "/recent_projects.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray array;
        for (const QString &projectPath : m_recentProjects) {
            array.append(projectPath);
        }
        
        QJsonDocument doc(array);
        file.write(doc.toJson());
    }
}

void ProjectManager::loadProjectSettings()
{
    if (m_currentProjectPath.isEmpty()) {
        return;
    }
    
    QFile file(m_currentProjectPath + "/.qtcide_project");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        m_projectSettings = doc.object();
    }
}

void ProjectManager::saveProjectSettings()
{
    if (m_currentProjectPath.isEmpty()) {
        return;
    }
    
    QFile file(m_currentProjectPath + "/.qtcide_project");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_projectSettings);
        file.write(doc.toJson());
    }
}
