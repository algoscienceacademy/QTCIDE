#include "Terminal.h"
#include <QDir>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextCursor>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>

Terminal::Terminal(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
    , m_currentDirectory(QDir::homePath())
{
    setupUI();
    applyTerminalStyle();
    autoDetectTerminal();
    loadTerminalSettings();
    initializeTerminal();
    
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Terminal::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &Terminal::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &Terminal::onProcessError);
}

void Terminal::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);
    
    // Terminal output
    m_output = new QTextEdit;
    m_output->setReadOnly(true);
    m_output->setFont(QFont("Consolas", 10));
    
    // Command input area
    auto *inputLayout = new QHBoxLayout;
    
    m_promptLabel = new QLabel("$ ");
    m_promptLabel->setStyleSheet("color: #FF8C00; font-weight: bold; font-family: 'Consolas';");
    
    m_input = new QLineEdit;
    m_input->setFont(QFont("Consolas", 10));
    
    connect(m_input, &QLineEdit::returnPressed, this, &Terminal::executeCommand);
    
    inputLayout->addWidget(m_promptLabel);
    inputLayout->addWidget(m_input);
    
    layout->addWidget(m_output, 1);
    layout->addLayout(inputLayout);
    
    // Initial welcome message
    appendText("QTCIDE Terminal v1.0\n");
    appendText("Current directory: " + m_currentDirectory + "\n");
    appendText("Type 'help' for available commands\n\n");
    updatePrompt();
}

void Terminal::applyTerminalStyle()
{
    setStyleSheet(R"(
        Terminal {
            background: rgba(20, 20, 20, 220);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 8px;
        }
        
        QTextEdit {
            background: rgba(25, 25, 25, 200);
            border: 1px solid rgba(255, 140, 0, 50);
            border-radius: 4px;
            color: #ffffff;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        
        QLineEdit {
            background: rgba(30, 30, 30, 200);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 4px;
            color: #ffffff;
            padding: 4px 8px;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        
        QLineEdit:focus {
            border: 2px solid rgba(255, 140, 0, 150);
        }
        
        QScrollBar:vertical {
            background: rgba(40, 40, 40, 100);
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background: rgba(255, 140, 0, 150);
            border-radius: 6px;
            min-height: 20px;
        }
    )");
}

void Terminal::clear()
{
    m_output->clear();
}

void Terminal::appendText(const QString &text)
{
    m_output->moveCursor(QTextCursor::End);
    m_output->insertPlainText(text);
    m_output->moveCursor(QTextCursor::End);
    
    // Auto-scroll to bottom
    QScrollBar *scrollBar = m_output->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void Terminal::executeCommand()
{
    QString command = m_input->text().trimmed();
    if (command.isEmpty()) return;
    
    appendText(QString("%1 %2\n").arg(m_promptLabel->text()).arg(command));
    m_input->clear();
    
    // Handle built-in commands
    if (command == "help") {
        showHelp();
        return;
    }
    
    if (command == "clear") {
        clear();
        appendText("QTCIDE Terminal v1.0\n");
        appendText("Current directory: " + m_currentDirectory + "\n");
        appendText("Type 'help' for available commands\n\n");
        return;
    }
    
    if (command == "pwd") {
        appendText(m_currentDirectory + "\n\n");
        return;
    }
    
    if (command == "tree") {
        showDirectoryTree();
        return;
    }
    
    if (command.startsWith("find ")) {
        findFiles(command.mid(5).trimmed());
        return;
    }
    
    if (command.startsWith("cd ")) {
        changeDirectory(command.mid(3).trimmed());
        return;
    }
    
    if (command.startsWith("mkdir ")) {
        createDirectory(command.mid(6).trimmed());
        return;
    }
    
    if (command.startsWith("touch ") || command.startsWith("echo. > ")) {
        QString fileName = command.startsWith("touch ") ? command.mid(6).trimmed() : command.mid(8).trimmed();
        createFile(fileName);
        return;
    }
    
    if (command.startsWith("rm ") || command.startsWith("del ")) {
        QString fileName = command.startsWith("rm ") ? command.mid(3).trimmed() : command.mid(4).trimmed();
        deleteFile(fileName);
        return;
    }
    
    if (command.startsWith("rmdir ")) {
        deleteDirectory(command.mid(6).trimmed());
        return;
    }
    
    if (command == "ls" || command == "dir") {
        listDirectory();
        return;
    }
    
    if (command.startsWith("cat ") || command.startsWith("type ")) {
        QString fileName = command.startsWith("cat ") ? command.mid(4).trimmed() : command.mid(5).trimmed();
        showFileContent(fileName);
        return;
    }
    
    if (command.startsWith("git ")) {
        executeGitCommand(command);
        return;
    }
    
    if (command.startsWith("cmake ") || command.startsWith("ninja ") || command.startsWith("make ")) {
        executeBuildCommand(command);
        return;
    }
    
    // Execute system command
    executeSystemCommand(command);
}

void Terminal::setCurrentDirectory(const QString &path)
{
    if (QDir(path).exists()) {
        m_currentDirectory = path;
        updatePrompt();
        emit directoryChanged(m_currentDirectory);
    }
}

void Terminal::showHelp()
{
    appendText("Available commands:\n");
    appendText("  help          - Show this help\n");
    appendText("  clear         - Clear terminal\n");
    appendText("  pwd           - Print working directory\n");
    appendText("  cd <dir>      - Change directory\n");
    appendText("  ls/dir        - List directory contents\n");
    appendText("  mkdir <name>  - Create directory (supports nested paths)\n");
    appendText("  touch <file>  - Create empty file (supports nested paths)\n");
    appendText("  rm <file>     - Delete file\n");
    appendText("  rmdir <dir>   - Delete directory\n");
    appendText("  cat <file>    - Show file content\n");
    appendText("  tree          - Show directory tree\n");
    appendText("  find <name>   - Find files/folders\n");
    appendText("  git <cmd>     - Git commands\n");
    appendText("  cmake <args>  - CMake build commands\n");
    appendText("  ninja <args>  - Ninja build commands\n");
    appendText("  make <args>   - Make build commands\n");
    appendText("  Any system command\n\n");
}

void Terminal::changeDirectory(const QString &path)
{
    QString newPath = path;
    
    if (newPath == "..") {
        QDir dir(m_currentDirectory);
        if (dir.cdUp()) {
            m_currentDirectory = dir.absolutePath();
        }
    } else if (newPath == "~") {
        m_currentDirectory = QDir::homePath();
    } else {
        if (!QDir::isAbsolutePath(newPath)) {
            newPath = QDir(m_currentDirectory).absoluteFilePath(newPath);
        }
        
        QDir dir(newPath);
        if (dir.exists()) {
            m_currentDirectory = dir.absolutePath();
        } else {
            appendText(QString("Directory not found: %1\n").arg(newPath));
            updatePrompt();
            return;
        }
    }
    
    appendText(QString("Changed to: %1\n").arg(m_currentDirectory));
    updatePrompt();
    emit directoryChanged(m_currentDirectory);
}

void Terminal::createDirectory(const QString &name)
{
    QDir dir;
    QString fullPath = QDir(m_currentDirectory).absoluteFilePath(name);
    
    if (dir.mkpath(fullPath)) {
        appendText(QString("Directory created: %1\n").arg(name));
        emit fileSystemChanged();
    } else {
        appendText(QString("Failed to create directory: %1\n").arg(name));
    }
    appendText("\n");
}

void Terminal::createFile(const QString &fileName)
{
    QString filePath = QDir(m_currentDirectory).absoluteFilePath(fileName);
    
    // Create parent directories if they don't exist
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.dir().absolutePath());
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        appendText(QString("File created: %1\n").arg(fileName));
        emit fileSystemChanged();
    } else {
        appendText(QString("Failed to create file: %1\n").arg(fileName));
    }
    appendText("\n");
}

void Terminal::deleteFile(const QString &fileName)
{
    QString filePath = QDir(m_currentDirectory).absoluteFilePath(fileName);
    if (QFile::remove(filePath)) {
        appendText(QString("File deleted: %1\n").arg(fileName));
        emit fileSystemChanged();
    } else {
        appendText(QString("Failed to delete file: %1\n").arg(fileName));
    }
    appendText("\n");
}

void Terminal::deleteDirectory(const QString &dirName)
{
    QString dirPath = QDir(m_currentDirectory).absoluteFilePath(dirName);
    QDir dir(dirPath);
    if (dir.removeRecursively()) {
        appendText(QString("Directory deleted: %1\n").arg(dirName));
        emit fileSystemChanged();
    } else {
        appendText(QString("Failed to delete directory: %1\n").arg(dirName));
    }
    appendText("\n");
}

void Terminal::listDirectory()
{
    QDir dir(m_currentDirectory);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
    
    for (const QFileInfo &info : entries) {
        QString line;
        if (info.isDir()) {
            line = QString("[DIR]  %1\n").arg(info.fileName());
        } else {
            line = QString("[FILE] %1 (%2 bytes)\n").arg(info.fileName()).arg(info.size());
        }
        appendText(line);
    }
    appendText("\n");
}

void Terminal::showFileContent(const QString &fileName)
{
    QString filePath = QDir(m_currentDirectory).absoluteFilePath(fileName);
    QFile file(filePath);
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        appendText(content + "\n\n");
    } else {
        appendText(QString("Failed to read file: %1\n\n").arg(fileName));
    }
}

void Terminal::executeGitCommand(const QString &command)
{
    appendText("Executing: " + command + "\n");
    m_process->setWorkingDirectory(m_currentDirectory);
    
    QStringList args = command.split(' ', Qt::SkipEmptyParts);
    args.removeFirst(); // Remove "git"
    
    m_process->start("git", args);
    
    if (!m_process->waitForStarted()) {
        appendText("Failed to start git command\n\n");
    }
}

void Terminal::executeBuildCommand(const QString &command)
{
    appendText("Executing: " + command + "\n");
    m_process->setWorkingDirectory(m_currentDirectory);
    
    // Set up environment based on current terminal type
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString currentPath = env.value("PATH");
    
#ifdef Q_OS_WIN
    // Add appropriate paths based on terminal type
    if (m_shellType == "msys2") {
        QString newPath = "C:/msys64/usr/bin;C:/msys64/mingw64/bin;" + currentPath;
        env.insert("PATH", newPath);
        env.insert("MSYSTEM", "MSYS");
    } else if (m_shellType == "mingw64") {
        QString newPath = "C:/msys64/mingw64/bin;C:/msys64/usr/bin;" + currentPath;
        env.insert("PATH", newPath);
        env.insert("MSYSTEM", "MINGW64");
    }
#endif
    
    m_process->setProcessEnvironment(env);
    
    QStringList args = command.split(' ', Qt::SkipEmptyParts);
    QString program = args.takeFirst();
    
    m_process->start(program, args);
    
    if (!m_process->waitForStarted()) {
        appendText(QString("Failed to start %1 command\n").arg(program));
        appendText("Make sure the tool is installed and in PATH\n");
        appendText("Current terminal: " + m_shellType + "\n\n");
    }
}

void Terminal::autoDetectTerminal()
{
    m_availableTerminals = detectAvailableTerminals();
    
    if (m_availableTerminals.isEmpty()) {
        // Fallback to system default
#ifdef Q_OS_WIN
        m_shellType = "cmd";
#elif defined(Q_OS_MACOS)
        m_shellType = "zsh";
#else
        m_shellType = "bash";
#endif
    } else {
        // Use the first available terminal as default
        m_shellType = m_availableTerminals.first();
    }
    
    appendText(QString("Auto-detected terminal: %1\n").arg(m_shellType));
    appendText(QString("Available terminals: %1\n\n").arg(m_availableTerminals.join(", ")));
}

QStringList Terminal::detectAvailableTerminals()
{
    QStringList available;
    
#ifdef Q_OS_WIN
    QStringList candidates = {"cmd", "powershell", "pwsh", "msys2", "mingw64", "gitbash"};
    
    for (const QString &terminal : candidates) {
        if (isTerminalAvailable(terminal)) {
            available << terminal;
        }
    }
#elif defined(Q_OS_MACOS)
    QStringList candidates = {"zsh", "bash", "fish"};
    
    for (const QString &terminal : candidates) {
        if (isTerminalAvailable(terminal)) {
            available << terminal;
        }
    }
#else // Linux
    QStringList candidates = {"bash", "zsh", "fish", "dash"};
    
    for (const QString &terminal : candidates) {
        if (isTerminalAvailable(terminal)) {
            available << terminal;
        }
    }
#endif
    
    return available;
}

bool Terminal::isTerminalAvailable(const QString &terminalType)
{
    QString executablePath = getTerminalExecutablePath(terminalType);
    
    if (executablePath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(executablePath);
    return fileInfo.exists() && fileInfo.isExecutable();
}

QString Terminal::getTerminalExecutablePath(const QString &terminalType) const
{
#ifdef Q_OS_WIN
    if (terminalType == "cmd") {
        return QStandardPaths::findExecutable("cmd");
    } else if (terminalType == "powershell") {
        return QStandardPaths::findExecutable("powershell");
    } else if (terminalType == "pwsh") {
        return QStandardPaths::findExecutable("pwsh");
    } else if (terminalType == "msys2") {
        QStringList paths = {"C:/msys64/usr/bin/bash.exe", "C:/msys64/usr/bin/bash"};
        for (const QString &path : paths) {
            if (QFileInfo::exists(path)) return path;
        }
    } else if (terminalType == "mingw64") {
        QStringList paths = {"C:/msys64/mingw64/bin/bash.exe", "C:/msys64/mingw64/bin/bash"};
        for (const QString &path : paths) {
            if (QFileInfo::exists(path)) return path;
        }
    } else if (terminalType == "gitbash") {
        QStringList paths = {
            "C:/Program Files/Git/bin/bash.exe",
            "C:/Program Files (x86)/Git/bin/bash.exe",
            "C:/Git/bin/bash.exe"
        };
        for (const QString &path : paths) {
            if (QFileInfo::exists(path)) return path;
        }
    }
#else
    if (terminalType == "bash") {
        return QStandardPaths::findExecutable("bash");
    } else if (terminalType == "zsh") {
        return QStandardPaths::findExecutable("zsh");
    } else if (terminalType == "fish") {
        return QStandardPaths::findExecutable("fish");
    } else if (terminalType == "dash") {
        return QStandardPaths::findExecutable("dash");
    }
#endif
    
    return QString();
}

QString Terminal::getTerminalDisplayName() const
{
    if (m_shellType == "cmd") return "Command Prompt";
    if (m_shellType == "powershell") return "PowerShell";
    if (m_shellType == "pwsh") return "PowerShell Core";
    if (m_shellType == "msys2") return "MSYS2 Bash";
    if (m_shellType == "mingw64") return "MinGW64 Bash";
    if (m_shellType == "gitbash") return "Git Bash";
    if (m_shellType == "bash") return "Bash";
    if (m_shellType == "zsh") return "Zsh";
    if (m_shellType == "fish") return "Fish Shell";
    if (m_shellType == "dash") return "Dash";
    if (m_shellType == "custom") return "Custom Shell";
    return m_shellType;
}

void Terminal::initializeTerminal()
{
    clear();
    appendText("QTCIDE Terminal v1.0\n");
    appendText("Terminal Type: " + m_shellType + "\n");
    appendText("Current directory: " + m_currentDirectory + "\n");
    
    QSettings settings("QTCIDE", "Settings");
    if (settings.value("Terminal/ClearOnStartup", false).toBool()) {
        QTimer::singleShot(100, this, [this]() {
            clear();
            updatePrompt();
        });
    } else {
        appendText("Type 'help' for available commands\n\n");
        updatePrompt();
    }
}

void Terminal::loadTerminalSettings()
{
    QSettings settings("QTCIDE", "Settings");
    QString savedTerminalType = settings.value("Terminal/Type", "").toString();
    QString customShell = settings.value("Terminal/CustomShell", "").toString();
    QString startupCommands = settings.value("Terminal/StartupCommands", "").toString();
    int fontSize = settings.value("Terminal/FontSize", 10).toInt();
    QString fontFamily = settings.value("Terminal/FontFamily", "Consolas").toString();
    
    // Only use saved terminal type if it's available
    if (!savedTerminalType.isEmpty() && 
        (m_availableTerminals.contains(savedTerminalType) || savedTerminalType == "custom")) {
        m_shellType = savedTerminalType;
    }
    
    m_customShellPath = customShell;
    
    // Apply font settings
    QFont font(fontFamily, fontSize);
    m_output->setFont(font);
    m_input->setFont(font);
    
    // Run startup commands
    if (!startupCommands.isEmpty()) {
        QStringList commands = startupCommands.split(';', Qt::SkipEmptyParts);
        for (const QString &cmd : commands) {
            QString trimmedCmd = cmd.trimmed();
            if (!trimmedCmd.isEmpty()) {
                appendText("$ " + trimmedCmd + "\n");
                // Execute startup command after a small delay
                QTimer::singleShot(500, this, [this, trimmedCmd]() {
                    executeSystemCommand(trimmedCmd);
                });
            }
        }
    }
    
    appendText(QString("Terminal switched to: %1\n\n").arg(m_shellType));
}

void Terminal::applyTerminalSettings()
{
    QSettings settings("QTCIDE", "Settings");
    QString newTerminalType = settings.value("Terminal/Type", "").toString();
    QString customShell = settings.value("Terminal/CustomShell", "").toString();
    int fontSize = settings.value("Terminal/FontSize", 10).toInt();
    QString fontFamily = settings.value("Terminal/FontFamily", "Consolas").toString();
    
    // Check if terminal type changed
    bool terminalTypeChanged = false;
    if (!newTerminalType.isEmpty() && newTerminalType != m_shellType) {
        if (newTerminalType == "custom" || m_availableTerminals.contains(newTerminalType)) {
            switchToTerminal(newTerminalType);
            terminalTypeChanged = true;
        }
    }
    
    m_customShellPath = customShell;
    
    // Apply font changes
    QFont font(fontFamily, fontSize);
    m_output->setFont(font);
    m_input->setFont(font);
    
    if (terminalTypeChanged) {
        appendText(QString("=== Terminal switched to: %1 ===\n").arg(getTerminalDisplayName()));
        updatePrompt();
    }
}

void Terminal::switchToTerminal(const QString &terminalType)
{
    if (terminalType == "custom" || m_availableTerminals.contains(terminalType)) {
        m_shellType = terminalType;
        
        // Kill any running process
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(1000);
        }
        
        appendText(QString("Switched to %1 terminal\n").arg(getTerminalDisplayName()));
        updatePrompt();
    } else {
        appendText(QString("Terminal type '%1' is not available\n").arg(terminalType));
    }
}

QString Terminal::getShellExecutable() const
{
    if (m_shellType == "custom" && !m_customShellPath.isEmpty()) {
        return m_customShellPath;
    }
    return getTerminalExecutablePath(m_shellType);
}

void Terminal::updatePrompt()
{
    QString prompt;
    
    if (m_shellType == "cmd") {
        prompt = QDir::toNativeSeparators(m_currentDirectory) + ">";
    } else if (m_shellType == "powershell" || m_shellType == "pwsh") {
        prompt = "PS " + QDir::toNativeSeparators(m_currentDirectory) + ">";
    } else {
        // Unix-like shells
        QString dirName = QFileInfo(m_currentDirectory).baseName();
        if (dirName.isEmpty()) dirName = "/";
        prompt = QString("%1@qtcide:%2$ ").arg(qgetenv("USER").isEmpty() ? "user" : qgetenv("USER")).arg(dirName);
    }
    
    m_promptLabel->setText(prompt);
}

void Terminal::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        appendText("Process crashed\n\n");
    } else {
        appendText(QString("Process finished with exit code: %1\n\n").arg(exitCode));
    }
}

void Terminal::onProcessOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    appendText(QString::fromLocal8Bit(data));
}

void Terminal::onProcessError()
{
    QByteArray data = m_process->readAllStandardError();
    appendText(QString::fromLocal8Bit(data));
}

void Terminal::showDirectoryTree()
{
    appendText("Directory tree:\n");
    showTree(m_currentDirectory, 0);
    appendText("\n");
}

void Terminal::showTree(const QString &path, int depth)
{
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name);
    
    QString indent = QString("  ").repeated(depth);
    
    for (const QFileInfo &info : entries) {
        QString line = indent;
        if (info.isDir()) {
            line += "📁 " + info.fileName() + "/\n";
            appendText(line);
            if (depth < 3) { // Limit depth to avoid too much output
                showTree(info.absoluteFilePath(), depth + 1);
            }
        } else {
            line += "📄 " + info.fileName() + "\n";
            appendText(line);
        }
    }
}

void Terminal::findFiles(const QString &pattern)
{
    if (pattern.isEmpty()) {
        appendText("Usage: find <pattern>\n\n");
        return;
    }
    
    appendText(QString("Searching for: %1\n").arg(pattern));
    
    QDirIterator it(m_currentDirectory, QStringList() << "*" + pattern + "*",
                    QDir::AllEntries | QDir::NoDotAndDotDot, 
                    QDirIterator::Subdirectories);
    
    int count = 0;
    while (it.hasNext()) {
        QString filePath = it.next();
        QString relativePath = QDir(m_currentDirectory).relativeFilePath(filePath);
        QFileInfo info(filePath);
        
        if (info.isDir()) {
            appendText(QString("📁 %1/\n").arg(relativePath));
        } else {
            appendText(QString("📄 %1\n").arg(relativePath));
        }
        count++;
        
        if (count > 50) { // Limit results
            appendText("... (showing first 50 results)\n");
            break;
        }
    }
    
    if (count == 0) {
        appendText("No files found.\n");
    } else {
        appendText(QString("Found %1 items.\n").arg(count));
    }
    appendText("\n");
}

void Terminal::executeSystemCommand(const QString &command)
{
    m_process->setWorkingDirectory(m_currentDirectory);
    
    QStringList args;
    QString program;
    
    // Get the actual executable path for the current shell type
    QString shellExecutable = getTerminalExecutablePath(m_shellType);
    
    // Handle custom shell
    if (m_shellType == "custom" && !m_customShellPath.isEmpty()) {
        program = m_customShellPath;
        args << "-c" << command;
    } else if (!shellExecutable.isEmpty()) {
        program = shellExecutable;
        
        // Determine shell arguments based on type
        if (m_shellType == "cmd") {
            args << "/c" << command;
        } else if (m_shellType == "powershell") {
            args << "-NoProfile" << "-Command" << command;
        } else if (m_shellType == "pwsh") {
            args << "-NoProfile" << "-Command" << command;
        } else {
            // Unix-like shells (bash, zsh, fish, msys2, mingw64, gitbash, etc.)
            args << "-c" << command;
        }
    } else {
        // Direct command execution for simple commands
        QStringList commandParts = command.split(' ', Qt::SkipEmptyParts);
        if (!commandParts.isEmpty()) {
            program = commandParts.first();
            args = commandParts.mid(1);
        } else {
            appendText("Error: Empty command\n\n");
            return;
        }
    }
    
    if (program.isEmpty()) {
        appendText("Error: No executable found for command: " + command + "\n\n");
        return;
    }
    
    // Set environment variables for better PATH resolution
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString currentPath = env.value("PATH");
    QStringList additionalPaths;
    
#ifdef Q_OS_WIN
    // Add appropriate paths based on terminal type
    if (m_shellType == "msys2") {
        additionalPaths << "C:/msys64/usr/bin" << "C:/msys64/mingw64/bin";
    } else if (m_shellType == "mingw64") {
        additionalPaths << "C:/msys64/mingw64/bin" << "C:/msys64/usr/bin";
    } else if (m_shellType == "gitbash") {
        additionalPaths << "C:/Program Files/Git/bin" << "C:/Program Files (x86)/Git/bin" << "C:/Git/bin";
    }
#else
    // Add common Unix paths
    if (m_shellType == "bash") {
        additionalPaths << "/usr/local/bin" << "/usr/bin" << "/bin";
    } else if (m_shellType == "zsh") {
        additionalPaths << "/usr/local/bin" << "/usr/bin" << "/bin";
    } else if (m_shellType == "fish") {
        additionalPaths << "/usr/local/bin" << "/usr/bin" << "/bin";
    } else if (m_shellType == "dash") {
        additionalPaths << "/usr/bin" << "/bin";
    }
#endif
    
    // Prepend additional paths to current PATH
    if (!additionalPaths.isEmpty()) {
#ifdef Q_OS_WIN
        QString newPath = (additionalPaths.join(";") + ";" + currentPath).replace("/", "\\");
#else
        QString newPath = additionalPaths.join(":") + ":" + currentPath;
#endif
        env.insert("PATH", newPath);
    }
    
    m_process->setProcessEnvironment(env);
    m_process->start(program, args);
    
    if (!m_process->waitForStarted(3000)) {
        appendText("Failed to execute command: " + command + "\n");
        appendText("Program: " + program + "\n");
        appendText("Arguments: " + args.join(" ") + "\n");
        appendText("Shell type: " + m_shellType + "\n");
        appendText("Shell executable: " + shellExecutable + "\n");
        appendText("Make sure the command is installed and in PATH\n\n");
    }
}
