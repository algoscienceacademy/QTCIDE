#include "MainWindow.h"
#include "WelcomeScreen.h"
#include "Terminal.h"
#include "CodeEditor.h"
#include "ProjectManager.h"
#include "NewProjectDialog.h"
#include "SettingsDialog.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFont>
#include <QFontDatabase>
#include <QDir>
#include <QFile>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_buildProcess(new QProcess(this))
    , m_runProcess(new QProcess(this))
    , m_projectManager(new ProjectManager(this))
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    applyGlassmorphicStyle();
    
    // Connect build and run processes
    connect(m_buildProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onBuildFinished);
    connect(m_buildProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onBuildOutput);
    connect(m_buildProcess, &QProcess::readyReadStandardError, this, &MainWindow::onBuildError);
    
    connect(m_runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onRunFinished);
    connect(m_runProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onRunOutput);
    connect(m_runProcess, &QProcess::readyReadStandardError, this, &MainWindow::onRunError);
    
    // Connect project manager
    connect(m_projectManager, &ProjectManager::projectOpened, this, &MainWindow::onProjectOpened);
    connect(m_projectManager, &ProjectManager::projectClosed, this, &MainWindow::onProjectClosed);
    
    resize(1400, 900);
    setWindowTitle("QTCIDE - Professional Qt IDE");
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    auto *layout = new QVBoxLayout(m_centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_stackedWidget = new QStackedWidget;
    layout->addWidget(m_stackedWidget);
    
    // Welcome screen
    m_welcomeScreen = new WelcomeScreen(this);
    m_stackedWidget->addWidget(m_welcomeScreen);
    
    // Main IDE interface
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_stackedWidget->addWidget(m_mainSplitter);
    
    // File explorer
    m_fileTree = new QTreeView;
    m_fileModel = new QFileSystemModel;
    m_fileModel->setRootPath(QDir::homePath());
    m_fileTree->setModel(m_fileModel);
    m_fileTree->setRootIndex(m_fileModel->index(QDir::homePath()));
    m_fileTree->setMaximumWidth(250);
    m_fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_mainSplitter->addWidget(m_fileTree);
    
    // Connect file tree signals
    connect(m_fileTree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        QString filePath = m_fileModel->filePath(index);
        if (QFileInfo(filePath).isFile()) {
            openFileFromPath(filePath);
        }
    });
    
    // Context menu for file operations
    connect(m_fileTree, &QTreeView::customContextMenuRequested, this, &MainWindow::showFileContextMenu);
    
    // Right side splitter (editor + terminal)
    m_rightSplitter = new QSplitter(Qt::Vertical);
    m_mainSplitter->addWidget(m_rightSplitter);
    
    // Editor
    m_editor = new CodeEditor;
    m_rightSplitter->addWidget(m_editor);
    
    // Terminal
    m_terminal = new Terminal;
    m_terminal->setMaximumHeight(200);
    m_rightSplitter->addWidget(m_terminal);
    
    // Connect terminal signals
    connect(m_terminal, &Terminal::directoryChanged, this, [this](const QString &path) {
        if (!m_currentProjectPath.isEmpty()) {
            m_fileTree->setRootIndex(m_fileModel->index(path));
        }
    });
    
    connect(m_terminal, &Terminal::fileSystemChanged, this, [this]() {
        // Refresh file tree by updating the model
        QString currentPath = m_fileModel->rootPath();
        m_fileModel->setRootPath("");
        m_fileModel->setRootPath(currentPath);
    });
    
    m_mainSplitter->setSizes({250, 1150});
    m_rightSplitter->setSizes({700, 200});
    
    // Show welcome screen initially
    m_stackedWidget->setCurrentWidget(m_welcomeScreen);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New File", QKeySequence::New, this, &MainWindow::newFile);
    fileMenu->addAction("&Open File", QKeySequence::Open, this, &MainWindow::openFile);
    fileMenu->addAction("&Save File", QKeySequence::Save, this, &MainWindow::saveFile);
    fileMenu->addAction("Save &As...", QKeySequence::SaveAs, this, &MainWindow::saveAsFile);
    fileMenu->addSeparator();
    fileMenu->addAction("&New Project...", QKeySequence("Ctrl+Shift+N"), this, &MainWindow::newProject);
    fileMenu->addAction("&Open Project...", QKeySequence("Ctrl+Shift+O"), this, &MainWindow::openProject);
    fileMenu->addAction("Open &Folder", this, &MainWindow::openFolder);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", QKeySequence::Quit, this, &QWidget::close);
    
    auto *buildMenu = menuBar()->addMenu("&Build");
    buildMenu->addAction("&Configure", QKeySequence("Ctrl+Shift+C"), this, &MainWindow::configure);
    buildMenu->addAction("&Build", QKeySequence("Ctrl+B"), this, &MainWindow::build);
    buildMenu->addAction("&Rebuild", QKeySequence("Ctrl+Shift+B"), this, &MainWindow::rebuild);
    buildMenu->addAction("&Clean", this, &MainWindow::clean);
    buildMenu->addSeparator();
    buildMenu->addAction("&Run", QKeySequence("Ctrl+R"), this, &MainWindow::run);
    buildMenu->addAction("Run &Debug", QKeySequence("F5"), this, &MainWindow::runDebug);
    
    auto *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("&Welcome", this, &MainWindow::showWelcome);
    viewMenu->addAction("&Terminal", QKeySequence("Ctrl+`"), this, &MainWindow::focusTerminal);
    
    auto *toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("&Settings...", QKeySequence("Ctrl+,"), this, &MainWindow::showSettings);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar("Main");
    toolbar->addAction("New", this, &MainWindow::newFile);
    toolbar->addAction("Open", this, &MainWindow::openFile);
    toolbar->addAction("Save", this, &MainWindow::saveFile);
    toolbar->addAction("Folder", this, &MainWindow::openFolder);
    toolbar->addSeparator();
    toolbar->addAction("Build", this, &MainWindow::build);
    toolbar->addAction("Run", this, &MainWindow::run);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::applyGlassmorphicStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 #1a1a1a, stop: 1 #2d2d2d);
        }
        
        QMenuBar {
            background: rgba(40, 40, 40, 180);
            border: none;
            padding: 4px;
        }
        
        QMenuBar::item {
            background: transparent;
            padding: 8px 12px;
            border-radius: 4px;
            color: white;
        }
        
        QMenuBar::item:selected {
            background: rgba(255, 140, 0, 100);
        }
        
        QToolBar {
            background: rgba(50, 50, 50, 180);
            border: none;
            padding: 4px;
            spacing: 2px;
        }
        
        QTextEdit {
            background: rgba(30, 30, 30, 200);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 8px;
            color: white;
            selection-background-color: rgba(255, 140, 0, 100);
        }
        
        QTreeView {
            background: rgba(35, 35, 35, 200);
            border: 1px solid rgba(255, 140, 0, 50);
            border-radius: 8px;
            color: white;
        }
        
        QTreeView::item:selected {
            background: rgba(255, 140, 0, 100);
        }
        
        QSplitter::handle {
            background: rgba(255, 140, 0, 100);
        }
        
        QStatusBar {
            background: rgba(40, 40, 40, 180);
            color: white;
        }
    )");
}

void MainWindow::newFile()
{
    m_editor->clear();
    m_stackedWidget->setCurrentWidget(m_mainSplitter);
    statusBar()->showMessage("New file created");
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath());
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            m_editor->setPlainText(in.readAll());
            m_stackedWidget->setCurrentWidget(m_mainSplitter);
            statusBar()->showMessage("File opened: " + fileName);
        }
    }
}

void MainWindow::openFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!folderPath.isEmpty()) {
        m_currentProjectPath = folderPath;
        m_fileTree->setRootIndex(m_fileModel->index(folderPath));
        m_terminal->setCurrentDirectory(folderPath);
        m_stackedWidget->setCurrentWidget(m_mainSplitter);
        statusBar()->showMessage("Folder opened: " + folderPath);
    }
}

void MainWindow::configure()
{
    if (m_currentProjectPath.isEmpty()) {
        QMessageBox::warning(this, "Configure", "Please open a project folder first.");
        return;
    }
    
    m_terminal->clear();
    m_terminal->appendText("=== Configuring Project ===\n");
    m_terminal->appendText("Project: " + m_currentProjectPath + "\n");
    m_terminal->appendText("Terminal: " + m_terminal->getCurrentShellType() + "\n\n");
    
    QString buildDir = m_currentProjectPath + "/build";
    QDir().mkpath(buildDir);
    
    m_buildProcess->setWorkingDirectory(buildDir);
    statusBar()->showMessage("Configuring project...");
    
    // Use cmake directly instead of going through shell
    m_buildProcess->setProgram("cmake");
    QStringList args;
    args << ".." << "-G" << "Ninja";
    
#ifdef Q_OS_WIN
    args << "-DCMAKE_BUILD_TYPE=Release";
#endif
    
    m_buildProcess->setArguments(args);
    m_buildProcess->start();
    
    if (!m_buildProcess->waitForStarted()) {
        m_terminal->appendText("Error: Could not start cmake configure process\n");
        m_terminal->appendText("Make sure CMake is installed and in PATH\n\n");
        statusBar()->showMessage("Configure failed");
    }
}

void MainWindow::build()
{
    if (m_currentProjectPath.isEmpty()) {
        QMessageBox::warning(this, "Build", "Please open a project folder first.");
        return;
    }
    
    // Save current file before building
    if (!m_currentFilePath.isEmpty()) {
        saveFile();
    }
    
    m_terminal->clear();
    m_terminal->appendText("=== Building Project ===\n");
    m_terminal->appendText("Project: " + m_currentProjectPath + "\n");
    m_terminal->appendText("Terminal: " + m_terminal->getCurrentShellType() + "\n\n");
    
    QString buildDir = m_currentProjectPath + "/build";
    QDir().mkpath(buildDir);
    
    m_buildProcess->setWorkingDirectory(buildDir);
    
    // Check if CMakeLists.txt exists
    if (!QFile::exists(m_currentProjectPath + "/CMakeLists.txt")) {
        m_terminal->appendText("Error: No CMakeLists.txt found in project directory\n");
        m_terminal->appendText("Build failed.\n\n");
        return;
    }
    
    // Start build process - use cmake directly
    statusBar()->showMessage("Building project...");
    
    // Try cmake --build first
    m_buildProcess->setProgram("cmake");
    QStringList args;
    args << "--build" << ".";
    
#ifdef Q_OS_WIN
    args << "--config" << "Release";
#endif
    
    m_buildProcess->setArguments(args);
    m_buildProcess->start();
    
    if (!m_buildProcess->waitForStarted()) {
        // Fallback to ninja if cmake --build fails
        m_buildProcess->setProgram("ninja");
        m_buildProcess->setArguments(QStringList());
        m_buildProcess->start();
        
        if (!m_buildProcess->waitForStarted()) {
            m_terminal->appendText("Error: Could not start build process\n");
            m_terminal->appendText("Make sure CMake and Ninja are installed and in PATH\n\n");
            statusBar()->showMessage("Build failed - tools not found");
        }
    }
}

void MainWindow::rebuild()
{
    clean();
    QTimer::singleShot(1000, this, &MainWindow::configure);
    QTimer::singleShot(3000, this, &MainWindow::build);
}

void MainWindow::clean()
{
    if (m_currentProjectPath.isEmpty()) {
        QMessageBox::warning(this, "Clean", "Please open a project folder first.");
        return;
    }
    
    m_terminal->clear();
    m_terminal->appendText("=== Cleaning Project ===\n");
    
    QString buildDir = m_currentProjectPath + "/build";
    QDir dir(buildDir);
    if (dir.exists()) {
        dir.removeRecursively();
        m_terminal->appendText("Build directory cleaned.\n\n");
    } else {
        m_terminal->appendText("No build directory to clean.\n\n");
    }
    
    statusBar()->showMessage("Project cleaned");
}

void MainWindow::run()
{
    if (m_currentProjectPath.isEmpty()) {
        QMessageBox::warning(this, "Run", "Please open a project folder first.");
        return;
    }
    
    // Find executable in build directory
    QString buildDir = m_currentProjectPath + "/build";
    QDir dir(buildDir);
    
    QStringList nameFilters;
#ifdef Q_OS_WIN
    nameFilters << "*.exe";
#else
    nameFilters << "*";
#endif
    
    QFileInfoList executables = dir.entryInfoList(nameFilters, QDir::Files | QDir::Executable);
    
    QString executable;
    for (const QFileInfo &info : executables) {
        if (!info.fileName().contains("CMakeFiles") && !info.fileName().startsWith("cmake")) {
            executable = info.absoluteFilePath();
            break;
        }
    }
    
    if (executable.isEmpty()) {
        m_terminal->appendText("No executable found. Please build the project first.\n\n");
        statusBar()->showMessage("Run failed - no executable found");
        return;
    }
    
    m_terminal->appendText("=== Running Application ===\n");
    m_terminal->appendText("Executable: " + executable + "\n\n");
    
    m_runProcess->setWorkingDirectory(buildDir);
    m_runProcess->start(executable, QStringList());
    
    if (!m_runProcess->waitForStarted()) {
        m_terminal->appendText("Error: Could not start application\n\n");
        statusBar()->showMessage("Run failed");
    } else {
        statusBar()->showMessage("Application running...");
    }
}

void MainWindow::runDebug()
{
    // For now, just run normally - debug functionality can be added later
    run();
}

void MainWindow::newProject()
{
    NewProjectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString projectPath = dialog.fullProjectPath();
        QString projectName = dialog.projectName();
        QString projectType = dialog.projectType();
        
        if (m_projectManager->createProject(projectPath, projectName, projectType)) {
            // Open the created project properly
            m_currentProjectPath = projectPath;
            m_fileTree->setRootIndex(m_fileModel->index(projectPath));
            m_terminal->setCurrentDirectory(projectPath);
            m_stackedWidget->setCurrentWidget(m_mainSplitter);
            
            // Open the main source file in the editor
            QString mainFile;
            if (projectType == "Qt Application") {
                mainFile = projectPath + "/src/main.cpp";
            } else if (projectType == "Console Application") {
                mainFile = projectPath + "/src/main.cpp";
            } else if (projectType == "Static Library") {
                mainFile = projectPath + "/src/library.cpp";
            }
            
            if (!mainFile.isEmpty() && QFile::exists(mainFile)) {
                openFileFromPath(mainFile);
            }
            
            statusBar()->showMessage("Project created: " + projectName);
            setWindowTitle("QTCIDE - " + projectName);
            
            // Show welcome message in terminal
            m_terminal->clear();
            m_terminal->appendText("=== Project Created Successfully ===\n");
            m_terminal->appendText("Project: " + projectName + "\n");
            m_terminal->appendText("Type: " + projectType + "\n");
            m_terminal->appendText("Location: " + projectPath + "\n\n");
            m_terminal->appendText("To build this project:\n");
            m_terminal->appendText("1. Use Build -> Configure (Ctrl+Shift+C)\n");
            m_terminal->appendText("2. Use Build -> Build (Ctrl+B)\n");
            m_terminal->appendText("3. Use Build -> Run (Ctrl+R)\n\n");
        } else {
            QMessageBox::critical(this, "Error", "Failed to create project at: " + projectPath);
        }
    }
}

void MainWindow::openProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", QDir::homePath());
    if (!projectPath.isEmpty()) {
        if (m_projectManager->openProject(projectPath)) {
            openFolder();
        } else {
            QMessageBox::warning(this, "Error", "Failed to open project: " + projectPath);
        }
    }
}

void MainWindow::focusTerminal()
{
    m_terminal->setFocus();
}

void MainWindow::onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        m_terminal->appendText("Build process crashed\n");
        statusBar()->showMessage("Build failed - process crashed");
    } else if (exitCode == 0) {
        m_terminal->appendText("Build completed successfully\n\n");
        statusBar()->showMessage("Build successful");
    } else {
        m_terminal->appendText(QString("Build failed with exit code: %1\n\n").arg(exitCode));
        statusBar()->showMessage("Build failed");
    }
}

void MainWindow::onBuildOutput()
{
    QByteArray data = m_buildProcess->readAllStandardOutput();
    m_terminal->appendText(QString::fromLocal8Bit(data));
}

void MainWindow::onBuildError()
{
    QByteArray data = m_buildProcess->readAllStandardError();
    m_terminal->appendText(QString::fromLocal8Bit(data));
}

void MainWindow::onRunFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        m_terminal->appendText("Application crashed\n\n");
        statusBar()->showMessage("Application crashed");
    } else {
        m_terminal->appendText(QString("Application finished with exit code: %1\n\n").arg(exitCode));
        statusBar()->showMessage("Application finished");
    }
}

void MainWindow::onRunOutput()
{
    QByteArray data = m_runProcess->readAllStandardOutput();
    m_terminal->appendText(QString::fromLocal8Bit(data));
}

void MainWindow::onRunError()
{
    QByteArray data = m_runProcess->readAllStandardError();
    m_terminal->appendText(QString::fromLocal8Bit(data));
}

void MainWindow::onProjectOpened(const QString &projectPath)
{
    m_currentProjectPath = projectPath;
    m_fileTree->setRootIndex(m_fileModel->index(projectPath));
    m_terminal->setCurrentDirectory(projectPath);
    m_stackedWidget->setCurrentWidget(m_mainSplitter);
    
    QString projectName = QFileInfo(projectPath).baseName();
    setWindowTitle("QTCIDE - " + projectName);
    statusBar()->showMessage("Project opened: " + projectName);
}

void MainWindow::onProjectClosed()
{
    m_currentProjectPath.clear();
    setWindowTitle("QTCIDE - Professional Qt IDE");
    statusBar()->showMessage("Project closed");
}

void MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        saveAsFile();
    } else {
        saveFileToPath(m_currentFilePath);
    }
}

void MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::homePath());
    if (!fileName.isEmpty()) {
        saveFileToPath(fileName);
        m_currentFilePath = fileName;
    }
}

void MainWindow::showWelcome()
{
    m_stackedWidget->setCurrentWidget(m_welcomeScreen);
}

void MainWindow::openFileFromPath(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_editor->setPlainText(in.readAll());
        m_stackedWidget->setCurrentWidget(m_mainSplitter);
        m_currentFilePath = filePath;
        statusBar()->showMessage("File opened: " + filePath);
        setWindowTitle("QTCIDE - " + QFileInfo(filePath).fileName());
    }
}

void MainWindow::saveFileToPath(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_editor->toPlainText();
        statusBar()->showMessage("File saved: " + filePath);
        setWindowTitle("QTCIDE - " + QFileInfo(filePath).fileName());
    } else {
        QMessageBox::warning(this, "Save Error", "Could not save file: " + filePath);
    }
}

void MainWindow::showFileContextMenu(const QPoint &point)
{
    QModelIndex index = m_fileTree->indexAt(point);
    QString selectedPath;
    bool isDirectory = false;
    
    if (index.isValid()) {
        selectedPath = m_fileModel->filePath(index);
        isDirectory = m_fileModel->isDir(index);
    } else {
        selectedPath = m_fileModel->rootPath();
        isDirectory = true;
    }
    
    QMenu contextMenu(this);
    
    // File/Folder creation actions
    QAction *newFileAction = contextMenu.addAction("New File...");
    QAction *newFolderAction = contextMenu.addAction("New Folder...");
    contextMenu.addSeparator();
    
    // Open terminal action
    QAction *openTerminalAction = contextMenu.addAction("Open Terminal Here");
    
    if (index.isValid()) {
        contextMenu.addSeparator();
        
        // Rename and delete actions
        QAction *renameAction = contextMenu.addAction("Rename");
        QAction *deleteAction = contextMenu.addAction("Delete");
        
        // Copy path action
        contextMenu.addSeparator();
        QAction *copyPathAction = contextMenu.addAction("Copy Path");
        
        connect(renameAction, &QAction::triggered, [this, index]() {
            renameFileOrFolder(index);
        });
        
        connect(deleteAction, &QAction::triggered, [this, index]() {
            deleteFileOrFolder(index);
        });
        
        connect(copyPathAction, &QAction::triggered, [this, selectedPath]() {
            QApplication::clipboard()->setText(selectedPath);
            statusBar()->showMessage("Path copied to clipboard");
        });
    }
    
    connect(newFileAction, &QAction::triggered, [this, selectedPath, isDirectory]() {
        QString basePath = isDirectory ? selectedPath : QFileInfo(selectedPath).dir().absolutePath();
        createNewFile(basePath);
    });
    
    connect(newFolderAction, &QAction::triggered, [this, selectedPath, isDirectory]() {
        QString basePath = isDirectory ? selectedPath : QFileInfo(selectedPath).dir().absolutePath();
        createNewFolder(basePath);
    });
    
    connect(openTerminalAction, &QAction::triggered, [this, selectedPath, isDirectory]() {
        QString targetPath = isDirectory ? selectedPath : QFileInfo(selectedPath).dir().absolutePath();
        m_terminal->setCurrentDirectory(targetPath);
        focusTerminal();
    });
    
    contextMenu.exec(m_fileTree->mapToGlobal(point));
}

void MainWindow::createNewFile(const QString &basePath)
{
    bool ok;
    QString fileName = QInputDialog::getText(this, "New File", 
                                           "Enter file name:", QLineEdit::Normal, 
                                           "newfile.cpp", &ok);
    
    if (ok && !fileName.isEmpty()) {
        QString fullPath = QDir(basePath).absoluteFilePath(fileName);
        
        // Check if file already exists
        if (QFile::exists(fullPath)) {
            QMessageBox::warning(this, "File Exists", 
                                "A file with this name already exists.");
            return;
        }
        
        // Create directories if needed (for nested paths)
        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.dir().absolutePath());
        
        // Create the file
        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // Add basic content based on file extension
            QTextStream out(&file);
            QString ext = fileInfo.suffix().toLower();
            
            if (ext == "cpp" || ext == "cxx" || ext == "cc") {
                out << "#include <iostream>\n\n";
                out << "int main()\n";
                out << "{\n";
                out << "    // TODO: Add your code here\n";
                out << "    return 0;\n";
                out << "}\n";
            } else if (ext == "h" || ext == "hpp") {
                QString guard = fileInfo.baseName().toUpper() + "_H";
                out << "#ifndef " << guard << "\n";
                out << "#define " << guard << "\n\n";
                out << "// TODO: Add your declarations here\n\n";
                out << "#endif // " << guard << "\n";
            } else if (ext == "cmake" || fileInfo.fileName() == "CMakeLists.txt") {
                out << "# CMake configuration\n";
                out << "cmake_minimum_required(VERSION 3.16)\n\n";
                out << "# TODO: Add your CMake commands here\n";
            }
            
            file.close();
            statusBar()->showMessage("File created: " + fileName);
            
            // Open the new file in the editor
            openFileFromPath(fullPath);
        } else {
            QMessageBox::critical(this, "Error", 
                                "Failed to create file: " + fileName);
        }
    }
}

void MainWindow::createNewFolder(const QString &basePath)
{
    bool ok;
    QString folderName = QInputDialog::getText(this, "New Folder", 
                                             "Enter folder name:", QLineEdit::Normal, 
                                             "newfolder", &ok);
    
    if (ok && !folderName.isEmpty()) {
        QString fullPath = QDir(basePath).absoluteFilePath(folderName);
        
        // Create nested folders if path contains separators
        QDir dir;
        if (dir.mkpath(fullPath)) {
            statusBar()->showMessage("Folder created: " + folderName);
        } else {
            QMessageBox::critical(this, "Error", 
                                "Failed to create folder: " + folderName);
        }
    }
}

void MainWindow::renameFileOrFolder(const QModelIndex &index)
{
    QString currentPath = m_fileModel->filePath(index);
    QString currentName = m_fileModel->fileName(index);
    
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename", 
                                          "Enter new name:", QLineEdit::Normal, 
                                          currentName, &ok);
    
    if (ok && !newName.isEmpty() && newName != currentName) {
        QString parentPath = QFileInfo(currentPath).dir().absolutePath();
        QString newPath = QDir(parentPath).absoluteFilePath(newName);
        
        if (QFile::rename(currentPath, newPath)) {
            statusBar()->showMessage("Renamed: " + currentName + " -> " + newName);
        } else {
            QMessageBox::critical(this, "Error", 
                                "Failed to rename: " + currentName);
        }
    }
}

void MainWindow::deleteFileOrFolder(const QModelIndex &index)
{
    QString path = m_fileModel->filePath(index);
    QString name = m_fileModel->fileName(index);
    bool isDir = m_fileModel->isDir(index);
    
    QString message = isDir ? 
        QString("Are you sure you want to delete folder '%1' and all its contents?").arg(name) :
        QString("Are you sure you want to delete file '%1'?").arg(name);
    
    int ret = QMessageBox::question(this, "Confirm Delete", message,
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        bool success = false;
        
        if (isDir) {
            QDir dir(path);
            success = dir.removeRecursively();
        } else {
            success = QFile::remove(path);
        }
        
        if (success) {
            statusBar()->showMessage("Deleted: " + name);
        } else {
            QMessageBox::critical(this, "Error", 
                                "Failed to delete: " + name);
        }
    }
}

void MainWindow::showSettings()
{
    SettingsDialog dialog(this);
    
    // Connect to settings changes for immediate application
    connect(&dialog, &SettingsDialog::settingsChanged, this, [this]() {
        m_terminal->applyTerminalSettings();
        statusBar()->showMessage("Terminal settings applied: " + m_terminal->getCurrentShellType());
    });
    
    if (dialog.exec() == QDialog::Accepted) {
        // Apply settings one more time to ensure everything is updated
        m_terminal->applyTerminalSettings();
        statusBar()->showMessage("Settings updated and applied - using " + m_terminal->getCurrentShellType());
    }
}
