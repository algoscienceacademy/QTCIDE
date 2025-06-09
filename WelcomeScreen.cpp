#include "WelcomeScreen.h"
#include "MainWindow.h"
#include "NewProjectDialog.h"
#include "ProjectManager.h"
#include <QApplication>
#include <QFileDialog>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QDir>
#include <QMessageBox>

ProjectCard::ProjectCard(const QString &title, const QString &path, QWidget *parent)
    : QFrame(parent), m_path(path)
{
    setFixedSize(280, 160);
    setFrameStyle(QFrame::Box);
    
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    
    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    titleLabel->setWordWrap(true);
    
    auto *pathLabel = new QLabel(path);
    pathLabel->setStyleSheet("font-size: 12px; color: #cccccc;");
    pathLabel->setWordWrap(true);
    
    layout->addWidget(titleLabel);
    layout->addWidget(pathLabel);
    layout->addStretch();
    
    setStyleSheet(R"(
        ProjectCard {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(60, 60, 60, 180),
                                      stop: 1 rgba(80, 80, 80, 180));
            border: 2px solid rgba(255, 140, 0, 50);
            border-radius: 12px;
        }
        ProjectCard:hover {
            border: 2px solid rgba(255, 140, 0, 150);
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(70, 70, 70, 200),
                                      stop: 1 rgba(90, 90, 90, 200));
        }
    )");
    
    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 5);
    setGraphicsEffect(shadow);
    
    setCursor(Qt::PointingHandCursor);
}

void ProjectCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit projectClicked(m_path);
    }
    QFrame::mousePressEvent(event);
}

void ProjectCard::enterEvent(QEnterEvent *event)
{
    auto *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    QRect newGeometry = geometry();
    newGeometry.adjust(-5, -5, 5, 5);
    animation->setEndValue(newGeometry);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    
    QFrame::enterEvent(event);
}

void ProjectCard::leaveEvent(QEvent *event)
{
    auto *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(200);
    animation->setStartValue(geometry());
    QRect newGeometry = geometry();
    newGeometry.adjust(5, 5, -5, -5);
    animation->setEndValue(newGeometry);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    
    QFrame::leaveEvent(event);
}

WelcomeScreen::WelcomeScreen(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent), m_mainWindow(mainWindow)
{
    setupUI();
    setupRecentProjects();
    applyGlassmorphicStyle();
}

void WelcomeScreen::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // Header
    auto *headerLayout = new QVBoxLayout;
    
    auto *titleLabel = new QLabel("Welcome to QTCIDE");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        font-size: 36px;
        font-weight: bold;
        color: white;
        margin: 20px 0;
    )");
    
    auto *subtitleLabel = new QLabel("Professional Qt Development Environment");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(R"(
        font-size: 18px;
        color: #cccccc;
        margin-bottom: 30px;
    )");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    
    // Action buttons
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(20);
    
    auto *newProjectBtn = new QPushButton("Create New Project");
    auto *openProjectBtn = new QPushButton("Open Existing Project");
    
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 rgba(255, 140, 0, 180),
                                      stop: 1 rgba(255, 100, 0, 180));
            border: none;
            border-radius: 8px;
            color: white;
            font-size: 14px;
            font-weight: bold;
            padding: 12px 24px;
            min-width: 150px;
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
    )";
    
    newProjectBtn->setStyleSheet(buttonStyle);
    openProjectBtn->setStyleSheet(buttonStyle);
    
    connect(newProjectBtn, &QPushButton::clicked, this, &WelcomeScreen::createNewProject);
    connect(openProjectBtn, &QPushButton::clicked, this, &WelcomeScreen::openExistingProject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(newProjectBtn);
    buttonLayout->addWidget(openProjectBtn);
    buttonLayout->addStretch();
    
    // Recent projects section
    auto *recentLabel = new QLabel("Recent Projects");
    recentLabel->setStyleSheet(R"(
        font-size: 24px;
        font-weight: bold;
        color: white;
        margin: 30px 0 20px 0;
    )");
    
    m_scrollArea = new QScrollArea;
    m_contentWidget = new QWidget;
    m_projectsGrid = new QGridLayout(m_contentWidget);
    m_projectsGrid->setSpacing(20);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    m_mainLayout->addLayout(headerLayout);
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addWidget(recentLabel);
    m_mainLayout->addWidget(m_scrollArea, 1);
}

void WelcomeScreen::setupRecentProjects()
{
    // Add some sample recent projects
    QStringList recentProjects = {
        "My Qt Application",
        "Web Browser Project",
        "Game Engine",
        "Database Manager",
        "Image Editor",
        "Chat Application"
    };
    
    int row = 0, col = 0;
    const int maxCols = 3;
    
    for (const QString &project : recentProjects) {
        auto *card = new ProjectCard(project, "/path/to/" + project.toLower().replace(" ", "_"));
        connect(card, &ProjectCard::projectClicked, this, &WelcomeScreen::openRecentProject);
        
        m_projectsGrid->addWidget(card, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void WelcomeScreen::applyGlassmorphicStyle()
{
    setStyleSheet(R"(
        WelcomeScreen {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,
                                      stop: 0 #0a0a0a, stop: 0.5 #1a1a1a, stop: 1 #2a2a2a);
        }
        
        QScrollArea {
            background: transparent;
            border: none;
        }
        
        QScrollBar:vertical {
            background: rgba(50, 50, 50, 100);
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background: rgba(255, 140, 0, 150);
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: rgba(255, 140, 0, 200);
        }
    )");
}

void WelcomeScreen::createNewProject()
{
    if (m_mainWindow) {
        // Use the main window's newProject method for consistency
        m_mainWindow->newProject();
    }
}

void WelcomeScreen::openExistingProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(this, "Open Project", QDir::homePath());
    if (!projectPath.isEmpty()) {
        openRecentProject(projectPath);
    }
}

void WelcomeScreen::openRecentProject(const QString &path)
{
    if (m_mainWindow && QDir(path).exists()) {
        // Now we can call openFolder since it's public
        m_mainWindow->openFolder();
    }
}
