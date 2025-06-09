#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QRegularExpression>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    struct TerminalSettings {
        QString terminalType;
        QString customShellPath;
        QString startupCommands;
        int fontSize;
        QString fontFamily;
        bool clearOnStartup;
    };
    
    TerminalSettings getTerminalSettings() const;
    void setTerminalSettings(const TerminalSettings &settings);

signals:
    void settingsChanged();

private slots:
    void onTerminalTypeChanged();
    void browseShellPath();
    void resetToDefaults();
    void applySettings();

private:
    void setupUI();
    void setupTerminalTab();
    void setupEditorTab();
    void setupBuildTab();
    void applyGlassmorphicStyle();
    void loadSettings();
    void saveSettings();
    void populateTerminalTypes();
    
    QString getTerminalDisplayName(const QString &terminalType);
    
    QTabWidget *m_tabWidget;
    
    // Terminal settings
    QComboBox *m_terminalTypeCombo;
    QLineEdit *m_customShellEdit;
    QPushButton *m_browseShellButton;
    QLineEdit *m_startupCommandsEdit;
    QSpinBox *m_fontSizeSpinBox;
    QComboBox *m_fontFamilyCombo;
    QCheckBox *m_clearOnStartupCheck;
    
    // Editor settings
    QSpinBox *m_editorFontSizeSpinBox;
    QComboBox *m_editorFontFamilyCombo;
    QCheckBox *m_autoIndentCheck;
    QCheckBox *m_lineNumbersCheck;
    QCheckBox *m_syntaxHighlightingCheck;
    
    // Build settings
    QLineEdit *m_cmakePathEdit;
    QLineEdit *m_ninjaPathEdit;
    QLineEdit *m_gitPathEdit;
    QPushButton *m_browseCmakeButton;
    QPushButton *m_browseNinjaButton;
    QPushButton *m_browseGitButton;
    
    QPushButton *m_applyButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_resetButton;
    
    QSettings *m_settings;
};

#endif // SETTINGSDIALOG_H
