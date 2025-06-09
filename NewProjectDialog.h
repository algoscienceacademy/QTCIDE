#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QTextEdit>

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = nullptr);
    
    QString projectName() const;
    QString projectPath() const;
    QString projectType() const;
    QString fullProjectPath() const;

private slots:
    void browseProjectLocation();
    void onProjectTypeChanged();
    void validateInput();

private:
    void setupUI();
    void applyGlassmorphicStyle();
    
    QLineEdit *m_projectNameEdit;
    QLineEdit *m_projectLocationEdit;
    QComboBox *m_projectTypeCombo;
    QPushButton *m_browseButton;
    QPushButton *m_createButton;
    QPushButton *m_cancelButton;
    QTextEdit *m_descriptionText;
    QLabel *m_previewLabel;
};

#endif // NEWPROJECTDIALOG_H
