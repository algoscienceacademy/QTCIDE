#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

class MainWindow;

class ProjectCard : public QFrame
{
    Q_OBJECT
public:
    explicit ProjectCard(const QString &title, const QString &path, QWidget *parent = nullptr);

signals:
    void projectClicked(const QString &path);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString m_path;
};

class WelcomeScreen : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeScreen(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void createNewProject();
    void openExistingProject();
    void openRecentProject(const QString &path);

private:
    void setupUI();
    void setupRecentProjects();
    void applyGlassmorphicStyle();
    
    MainWindow *m_mainWindow;
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QGridLayout *m_projectsGrid;
};

#endif // WELCOMESCREEN_H
