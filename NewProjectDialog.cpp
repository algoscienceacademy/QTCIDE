#include "NewProjectDialog.h"
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

NewProjectDialog::NewProjectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    applyGlassmorphicStyle();
    setModal(true);
    setWindowTitle("Create New Project");
    setFixedSize(600, 500);
}

void NewProjectDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);
    
    // Title
    auto *titleLabel = new QLabel("Create New Project");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Project details group
    auto *detailsGroup = new QGroupBox("Project Details");
    auto *detailsLayout = new QFormLayout(detailsGroup);
    detailsLayout->setSpacing(15);
    
    // Project name
    m_projectNameEdit = new QLineEdit;
    m_projectNameEdit->setPlaceholderText("Enter project name...");
    detailsLayout->addRow("Project Name:", m_projectNameEdit);
    
    // Project location
    auto *locationLayout = new QHBoxLayout;
    m_projectLocationEdit = new QLineEdit;
    m_projectLocationEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    m_projectLocationEdit->setPlaceholderText("Project location...");
    
    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setMaximumWidth(100);
    
    locationLayout->addWidget(m_projectLocationEdit);
    locationLayout->addWidget(m_browseButton);
    detailsLayout->addRow("Location:", locationLayout);
    
    // Project type
    m_projectTypeCombo = new QComboBox;
    m_projectTypeCombo->addItems({
        "Qt Application",
        "Console Application", 
        "Static Library"
    });
    detailsLayout->addRow("Project Type:", m_projectTypeCombo);
    
    // Project path preview
    m_previewLabel = new QLabel;
    m_previewLabel->setStyleSheet("color: #cccccc; font-style: italic;");
    detailsLayout->addRow("Full Path:", m_previewLabel);
    
    // Description
    auto *descGroup = new QGroupBox("Description");
    auto *descLayout = new QVBoxLayout(descGroup);
    
    m_descriptionText = new QTextEdit;
    m_descriptionText->setMaximumHeight(100);
    m_descriptionText->setPlainText("A new project created with QTCIDE.");
    descLayout->addWidget(m_descriptionText);
    
    // Buttons
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel");
    m_createButton = new QPushButton("Create Project");
    m_createButton->setDefault(true);
    m_createButton->setEnabled(false);
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_createButton);
    
    // Add to main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(detailsGroup);
    mainLayout->addWidget(descGroup);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_browseButton, &QPushButton::clicked, this, &NewProjectDialog::browseProjectLocation);
    connect(m_projectTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NewProjectDialog::onProjectTypeChanged);
    connect(m_projectNameEdit, &QLineEdit::textChanged, this, &NewProjectDialog::validateInput);
    connect(m_projectLocationEdit, &QLineEdit::textChanged, this, &NewProjectDialog::validateInput);
    connect(m_createButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    onProjectTypeChanged();
    validateInput();
}

void NewProjectDialog::applyGlassmorphicStyle()
{
    setStyleSheet(R"(
        NewProjectDialog {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(20, 20, 20, 240),
                                      stop: 1 rgba(40, 40, 40, 240));
            border-radius: 12px;
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
        
        QLineEdit {
            background: rgba(50, 50, 50, 180);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 6px;
            color: white;
            padding: 8px;
            font-size: 12px;
        }
        
        QLineEdit:focus {
            border: 2px solid rgba(255, 140, 0, 150);
        }
        
        QComboBox {
            background: rgba(50, 50, 50, 180);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 6px;
            color: white;
            padding: 8px;
            font-size: 12px;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #FF8C00;
        }
        
        QTextEdit {
            background: rgba(50, 50, 50, 180);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 6px;
            color: white;
            padding: 8px;
            font-size: 12px;
        }
        
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 140, 0, 180),
                                      stop: 1 rgba(255, 100, 0, 180));
            border: none;
            border-radius: 6px;
            color: white;
            font-weight: bold;
            padding: 10px 20px;
            font-size: 12px;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 160, 0, 200),
                                      stop: 1 rgba(255, 120, 0, 200));
        }
        
        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 120, 0, 220),
                                      stop: 1 rgba(255, 80, 0, 220));
        }
        
        QPushButton:disabled {
            background: rgba(100, 100, 100, 100);
            color: rgba(255, 255, 255, 100);
        }
        
        QLabel {
            color: white;
            font-size: 12px;
        }
    )");
}

void NewProjectDialog::browseProjectLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project Location", 
                                                   m_projectLocationEdit->text());
    if (!dir.isEmpty()) {
        m_projectLocationEdit->setText(dir);
    }
}

void NewProjectDialog::onProjectTypeChanged()
{
    QString type = m_projectTypeCombo->currentText();
    QString description;
    
    if (type == "Qt Application") {
        description = "Creates a Qt application with main window, CMakeLists.txt, and basic project structure.";
    } else if (type == "Console Application") {
        description = "Creates a simple console application with main.cpp and CMakeLists.txt.";
    } else if (type == "Static Library") {
        description = "Creates a static library project with header and source files.";
    }
    
    m_descriptionText->setPlainText(description);
    validateInput();
}

void NewProjectDialog::validateInput()
{
    QString name = m_projectNameEdit->text().trimmed();
    QString location = m_projectLocationEdit->text().trimmed();
    
    bool valid = !name.isEmpty() && !location.isEmpty() && QDir(location).exists();
    m_createButton->setEnabled(valid);
    
    if (valid) {
        QString fullPath = QDir(location).absoluteFilePath(name);
        m_previewLabel->setText(fullPath);
    } else {
        m_previewLabel->setText("Invalid project name or location");
    }
}

QString NewProjectDialog::projectName() const
{
    return m_projectNameEdit->text().trimmed();
}

QString NewProjectDialog::projectPath() const
{
    return m_projectLocationEdit->text().trimmed();
}

QString NewProjectDialog::projectType() const
{
    return m_projectTypeCombo->currentText();
}

QString NewProjectDialog::fullProjectPath() const
{
    return QDir(projectPath()).absoluteFilePath(projectName());
}
