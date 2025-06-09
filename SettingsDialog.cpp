#include "SettingsDialog.h"
#include "Terminal.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QStandardItem>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_settings(new QSettings("QTCIDE", "Settings", this))
{
    setupUI();
    applyGlassmorphicStyle();
    loadSettings();
    setModal(true);
    setWindowTitle("QTCIDE Settings");
    setFixedSize(600, 500);
}

void SettingsDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Title
    auto *titleLabel = new QLabel("QTCIDE Settings");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Tab widget
    m_tabWidget = new QTabWidget;
    setupTerminalTab();
    setupEditorTab();
    setupBuildTab();
    mainLayout->addWidget(m_tabWidget);
    
    // Buttons
    auto *buttonLayout = new QHBoxLayout;
    m_resetButton = new QPushButton("Reset to Defaults");
    m_applyButton = new QPushButton("Apply");
    m_cancelButton = new QPushButton("Cancel");
    m_okButton = new QPushButton("OK");
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_terminalTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onTerminalTypeChanged);
    connect(m_browseShellButton, &QPushButton::clicked, this, &SettingsDialog::browseShellPath);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::applySettings);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::setupTerminalTab()
{
    auto *terminalWidget = new QWidget;
    auto *layout = new QVBoxLayout(terminalWidget);
    layout->setSpacing(15);
    
    // Terminal Type Group
    auto *terminalGroup = new QGroupBox("Terminal Configuration");
    auto *terminalLayout = new QFormLayout(terminalGroup);
    
    m_terminalTypeCombo = new QComboBox;
    populateTerminalTypes();
    terminalLayout->addRow("Terminal Type:", m_terminalTypeCombo);
    
    auto *customShellLayout = new QHBoxLayout;
    m_customShellEdit = new QLineEdit;
    m_customShellEdit->setPlaceholderText("Path to custom shell executable...");
    m_browseShellButton = new QPushButton("Browse...");
    m_browseShellButton->setMaximumWidth(80);
    customShellLayout->addWidget(m_customShellEdit);
    customShellLayout->addWidget(m_browseShellButton);
    terminalLayout->addRow("Custom Shell:", customShellLayout);
    
    m_startupCommandsEdit = new QLineEdit;
    m_startupCommandsEdit->setPlaceholderText("Commands to run on terminal startup...");
    terminalLayout->addRow("Startup Commands:", m_startupCommandsEdit);
    
    layout->addWidget(terminalGroup);
    
    // Appearance Group
    auto *appearanceGroup = new QGroupBox("Terminal Appearance");
    auto *appearanceLayout = new QFormLayout(appearanceGroup);
    
    m_fontFamilyCombo = new QComboBox;
    QFontDatabase fontDb;
    QStringList monospaceFonts = fontDb.families(QFontDatabase::Latin);
    monospaceFonts = monospaceFonts.filter(QRegularExpression("(Consolas|Courier|Monaco|Menlo|DejaVu|Liberation|Source Code|Fira Code|JetBrains)"));
    if (monospaceFonts.isEmpty()) {
        monospaceFonts << "Consolas" << "Courier New" << "monospace";
    }
    m_fontFamilyCombo->addItems(monospaceFonts);
    appearanceLayout->addRow("Font Family:", m_fontFamilyCombo);
    
    m_fontSizeSpinBox = new QSpinBox;
    m_fontSizeSpinBox->setRange(8, 24);
    m_fontSizeSpinBox->setValue(10);
    m_fontSizeSpinBox->setSuffix(" pt");
    appearanceLayout->addRow("Font Size:", m_fontSizeSpinBox);
    
    m_clearOnStartupCheck = new QCheckBox("Clear terminal on startup");
    appearanceLayout->addRow("", m_clearOnStartupCheck);
    
    layout->addWidget(appearanceGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(terminalWidget, "Terminal");
}

void SettingsDialog::setupEditorTab()
{
    auto *editorWidget = new QWidget;
    auto *layout = new QVBoxLayout(editorWidget);
    
    auto *editorGroup = new QGroupBox("Editor Settings");
    auto *editorLayout = new QFormLayout(editorGroup);
    
    m_editorFontFamilyCombo = new QComboBox;
    QFontDatabase fontDb;
    QStringList monospaceFonts = fontDb.families(QFontDatabase::Latin);
    monospaceFonts = monospaceFonts.filter(QRegularExpression("(Consolas|Courier|Monaco|Menlo|DejaVu|Liberation|Source Code|Fira Code|JetBrains)"));
    if (monospaceFonts.isEmpty()) {
        monospaceFonts << "Consolas" << "Courier New" << "monospace";
    }
    m_editorFontFamilyCombo->addItems(monospaceFonts);
    editorLayout->addRow("Font Family:", m_editorFontFamilyCombo);
    
    m_editorFontSizeSpinBox = new QSpinBox;
    m_editorFontSizeSpinBox->setRange(8, 24);
    m_editorFontSizeSpinBox->setValue(11);
    m_editorFontSizeSpinBox->setSuffix(" pt");
    editorLayout->addRow("Font Size:", m_editorFontSizeSpinBox);
    
    m_autoIndentCheck = new QCheckBox("Enable auto-indentation");
    editorLayout->addRow("", m_autoIndentCheck);
    
    m_lineNumbersCheck = new QCheckBox("Show line numbers");
    editorLayout->addRow("", m_lineNumbersCheck);
    
    m_syntaxHighlightingCheck = new QCheckBox("Enable syntax highlighting");
    editorLayout->addRow("", m_syntaxHighlightingCheck);
    
    layout->addWidget(editorGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(editorWidget, "Editor");
}

void SettingsDialog::setupBuildTab()
{
    auto *buildWidget = new QWidget;
    auto *layout = new QVBoxLayout(buildWidget);
    
    auto *buildGroup = new QGroupBox("Build Tools");
    auto *buildLayout = new QFormLayout(buildGroup);
    
    // CMake path
    auto *cmakeLayout = new QHBoxLayout;
    m_cmakePathEdit = new QLineEdit;
    m_cmakePathEdit->setPlaceholderText("Path to cmake executable...");
    m_browseCmakeButton = new QPushButton("Browse...");
    m_browseCmakeButton->setMaximumWidth(80);
    cmakeLayout->addWidget(m_cmakePathEdit);
    cmakeLayout->addWidget(m_browseCmakeButton);
    buildLayout->addRow("CMake Path:", cmakeLayout);
    
    // Ninja path
    auto *ninjaLayout = new QHBoxLayout;
    m_ninjaPathEdit = new QLineEdit;
    m_ninjaPathEdit->setPlaceholderText("Path to ninja executable...");
    m_browseNinjaButton = new QPushButton("Browse...");
    m_browseNinjaButton->setMaximumWidth(80);
    ninjaLayout->addWidget(m_ninjaPathEdit);
    ninjaLayout->addWidget(m_browseNinjaButton);
    buildLayout->addRow("Ninja Path:", ninjaLayout);
    
    // Git path
    auto *gitLayout = new QHBoxLayout;
    m_gitPathEdit = new QLineEdit;
    m_gitPathEdit->setPlaceholderText("Path to git executable...");
    m_browseGitButton = new QPushButton("Browse...");
    m_browseGitButton->setMaximumWidth(80);
    gitLayout->addWidget(m_gitPathEdit);
    gitLayout->addWidget(m_browseGitButton);
    buildLayout->addRow("Git Path:", gitLayout);
    
    layout->addWidget(buildGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(buildWidget, "Build Tools");
    
    // Connect browse buttons
    connect(m_browseCmakeButton, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select CMake Executable");
        if (!path.isEmpty()) m_cmakePathEdit->setText(path);
    });
    
    connect(m_browseNinjaButton, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select Ninja Executable");
        if (!path.isEmpty()) m_ninjaPathEdit->setText(path);
    });
    
    connect(m_browseGitButton, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select Git Executable");
        if (!path.isEmpty()) m_gitPathEdit->setText(path);
    });
}

void SettingsDialog::populateTerminalTypes()
{
    // Create a temporary terminal instance to detect available terminals
    Terminal tempTerminal;
    QStringList availableTerminals = tempTerminal.detectAvailableTerminals();
    
    m_terminalTypeCombo->clear();
    
    // Add available terminals with user-friendly names
    for (const QString &terminal : availableTerminals) {
        QString displayName = getTerminalDisplayName(terminal);
        m_terminalTypeCombo->addItem(displayName + " ✓", terminal);
    }
    
    // Always add custom option
    m_terminalTypeCombo->addItem("Custom Shell", "custom");
    
    // Add unavailable terminals as disabled items (for reference)
    QStringList allPossibleTerminals;
    
#ifdef Q_OS_WIN
    allPossibleTerminals << "cmd" << "powershell" << "pwsh" << "msys2" << "mingw64" << "gitbash";
#elif defined(Q_OS_MACOS)
    allPossibleTerminals << "zsh" << "bash" << "fish";
#else
    allPossibleTerminals << "bash" << "zsh" << "fish" << "dash";
#endif
    
    for (const QString &terminal : allPossibleTerminals) {
        if (!availableTerminals.contains(terminal)) {
            QString displayName = getTerminalDisplayName(terminal);
            int index = m_terminalTypeCombo->count();
            m_terminalTypeCombo->addItem(displayName + " (Not Available)", terminal);
            
            // Disable unavailable items
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_terminalTypeCombo->model());
            if (model) {
                QStandardItem* item = model->item(index);
                if (item) {
                    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                    item->setData(QVariant(QColor(128, 128, 128)), Qt::ForegroundRole);
                }
            }
        }
    }
}

QString SettingsDialog::getTerminalDisplayName(const QString &terminalType)
{
    if (terminalType == "cmd") return "Command Prompt";
    if (terminalType == "powershell") return "PowerShell";
    if (terminalType == "pwsh") return "PowerShell Core";
    if (terminalType == "msys2") return "MSYS2 Bash";
    if (terminalType == "mingw64") return "MinGW64 Bash";
    if (terminalType == "gitbash") return "Git Bash";
    if (terminalType == "bash") return "Bash";
    if (terminalType == "zsh") return "Zsh";
    if (terminalType == "fish") return "Fish Shell";
    if (terminalType == "dash") return "Dash";
    return terminalType;
}

void SettingsDialog::onTerminalTypeChanged()
{
    QString terminalType = m_terminalTypeCombo->currentData().toString();
    bool isCustom = (terminalType == "custom");
    
    m_customShellEdit->setEnabled(isCustom);
    m_browseShellButton->setEnabled(isCustom);
    
    if (!isCustom) {
        m_customShellEdit->clear();
    }
}

void SettingsDialog::browseShellPath()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Shell Executable");
    if (!path.isEmpty()) {
        m_customShellEdit->setText(path);
    }
}

void SettingsDialog::resetToDefaults()
{
    // Terminal defaults
    m_terminalTypeCombo->setCurrentIndex(0);
    m_customShellEdit->clear();
    m_startupCommandsEdit->clear();
    m_fontSizeSpinBox->setValue(10);
    m_fontFamilyCombo->setCurrentText("Consolas");
    m_clearOnStartupCheck->setChecked(false);
    
    // Editor defaults
    m_editorFontSizeSpinBox->setValue(11);
    m_editorFontFamilyCombo->setCurrentText("Consolas");
    m_autoIndentCheck->setChecked(true);
    m_lineNumbersCheck->setChecked(true);
    m_syntaxHighlightingCheck->setChecked(true);
    
    // Build tools defaults
    m_cmakePathEdit->setText("cmake");
    m_ninjaPathEdit->setText("ninja");
    m_gitPathEdit->setText("git");
}

void SettingsDialog::applySettings()
{
    saveSettings();
    
    // Emit signal to notify that settings changed
    emit settingsChanged();
    
    QString terminalType = m_terminalTypeCombo->currentData().toString();
    QMessageBox::information(this, "Settings", 
        QString("Settings applied successfully!\nTerminal switched to: %1")
        .arg(getTerminalDisplayName(terminalType)));
}

void SettingsDialog::loadSettings()
{
    // Terminal settings
    QString terminalType = m_settings->value("Terminal/Type", "").toString();
    
    // If no terminal type is saved, detect and use the first available
    if (terminalType.isEmpty()) {
        Terminal tempTerminal;
        QStringList available = tempTerminal.detectAvailableTerminals();
        if (!available.isEmpty()) {
            terminalType = available.first();
            m_settings->setValue("Terminal/Type", terminalType);
        }
    }
    
    // Set the combo box to the correct terminal type
    for (int i = 0; i < m_terminalTypeCombo->count(); ++i) {
        if (m_terminalTypeCombo->itemData(i).toString() == terminalType) {
            m_terminalTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    m_customShellEdit->setText(m_settings->value("Terminal/CustomShell", "").toString());
    m_startupCommandsEdit->setText(m_settings->value("Terminal/StartupCommands", "").toString());
    m_fontSizeSpinBox->setValue(m_settings->value("Terminal/FontSize", 10).toInt());
    m_fontFamilyCombo->setCurrentText(m_settings->value("Terminal/FontFamily", "Consolas").toString());
    m_clearOnStartupCheck->setChecked(m_settings->value("Terminal/ClearOnStartup", false).toBool());
    
    // Editor settings
    m_editorFontSizeSpinBox->setValue(m_settings->value("Editor/FontSize", 11).toInt());
    m_editorFontFamilyCombo->setCurrentText(m_settings->value("Editor/FontFamily", "Consolas").toString());
    m_autoIndentCheck->setChecked(m_settings->value("Editor/AutoIndent", true).toBool());
    m_lineNumbersCheck->setChecked(m_settings->value("Editor/LineNumbers", true).toBool());
    m_syntaxHighlightingCheck->setChecked(m_settings->value("Editor/SyntaxHighlighting", true).toBool());
    
    // Build tools settings
    m_cmakePathEdit->setText(m_settings->value("BuildTools/CMakePath", "cmake").toString());
    m_ninjaPathEdit->setText(m_settings->value("BuildTools/NinjaPath", "ninja").toString());
    m_gitPathEdit->setText(m_settings->value("BuildTools/GitPath", "git").toString());
    
    onTerminalTypeChanged();
}

void SettingsDialog::saveSettings()
{
    // Terminal settings
    m_settings->setValue("Terminal/Type", m_terminalTypeCombo->currentData().toString());
    m_settings->setValue("Terminal/CustomShell", m_customShellEdit->text());
    m_settings->setValue("Terminal/StartupCommands", m_startupCommandsEdit->text());
    m_settings->setValue("Terminal/FontSize", m_fontSizeSpinBox->value());
    m_settings->setValue("Terminal/FontFamily", m_fontFamilyCombo->currentText());
    m_settings->setValue("Terminal/ClearOnStartup", m_clearOnStartupCheck->isChecked());
    
    // Editor settings
    m_settings->setValue("Editor/FontSize", m_editorFontSizeSpinBox->value());
    m_settings->setValue("Editor/FontFamily", m_editorFontFamilyCombo->currentText());
    m_settings->setValue("Editor/AutoIndent", m_autoIndentCheck->isChecked());
    m_settings->setValue("Editor/LineNumbers", m_lineNumbersCheck->isChecked());
    m_settings->setValue("Editor/SyntaxHighlighting", m_syntaxHighlightingCheck->isChecked());
    
    // Build tools settings
    m_settings->setValue("BuildTools/CMakePath", m_cmakePathEdit->text());
    m_settings->setValue("BuildTools/NinjaPath", m_ninjaPathEdit->text());
    m_settings->setValue("BuildTools/GitPath", m_gitPathEdit->text());
    
    m_settings->sync();
}

SettingsDialog::TerminalSettings SettingsDialog::getTerminalSettings() const
{
    TerminalSettings settings;
    settings.terminalType = m_terminalTypeCombo->currentData().toString();
    settings.customShellPath = m_customShellEdit->text();
    settings.startupCommands = m_startupCommandsEdit->text();
    settings.fontSize = m_fontSizeSpinBox->value();
    settings.fontFamily = m_fontFamilyCombo->currentText();
    settings.clearOnStartup = m_clearOnStartupCheck->isChecked();
    return settings;
}

void SettingsDialog::setTerminalSettings(const TerminalSettings &settings)
{
    for (int i = 0; i < m_terminalTypeCombo->count(); ++i) {
        if (m_terminalTypeCombo->itemData(i).toString() == settings.terminalType) {
            m_terminalTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    m_customShellEdit->setText(settings.customShellPath);
    m_startupCommandsEdit->setText(settings.startupCommands);
    m_fontSizeSpinBox->setValue(settings.fontSize);
    m_fontFamilyCombo->setCurrentText(settings.fontFamily);
    m_clearOnStartupCheck->setChecked(settings.clearOnStartup);
}

void SettingsDialog::applyGlassmorphicStyle()
{
    setStyleSheet(R"(
        SettingsDialog {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(20, 20, 20, 240),
                                      stop: 1 rgba(40, 40, 40, 240));
            border-radius: 12px;
        }
        
        QTabWidget::pane {
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 8px;
            background: rgba(30, 30, 30, 180);
        }
        
        QTabBar::tab {
            background: rgba(50, 50, 50, 180);
            color: white;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        
        QTabBar::tab:selected {
            background: rgba(255, 140, 0, 150);
        }
        
        QGroupBox {
            font-weight: bold;
            color: white;
            border: 2px solid rgba(255, 140, 0, 100);
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QLineEdit, QComboBox, QSpinBox {
            background: rgba(50, 50, 50, 180);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 6px;
            color: white;
            padding: 6px;
            font-size: 11px;
        }
        
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
            border: 2px solid rgba(255, 140, 0, 150);
        }
        
        QCheckBox {
            color: white;
            font-size: 11px;
        }
        
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid rgba(255, 140, 0, 100);
            border-radius: 3px;
            background: rgba(50, 50, 50, 180);
        }
        
        QCheckBox::indicator:checked {
            background: rgba(255, 140, 0, 150);
        }
        
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 140, 0, 180),
                                      stop: 1 rgba(255, 100, 0, 180));
            border: none;
            border-radius: 6px;
            color: white;
            font-weight: bold;
            padding: 8px 16px;
            font-size: 11px;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 160, 0, 200),
                                      stop: 1 rgba(255, 120, 0, 200));
        }
        
        QLabel {
            color: white;
            font-size: 11px;
        }
    )");
}
