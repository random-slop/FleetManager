#include "ProjectDialog.h"
#include "ui_ProjectDialog.h"
#include "../database/FleetDatabase.h"
#include <QMessageBox>

ProjectDialog::ProjectDialog(QWidget *parent, const ProjectPtr& project)
    : QDialog(parent)
    , ui(new Ui::ProjectDialog)
    , m_project(project)
    , m_isEditMode(project != nullptr)
{
    ui->setupUi(this);
    setupUI();
    
    if (m_isEditMode) {
        fillFromProject(project);
    }
}

ProjectDialog::~ProjectDialog()
{
    delete ui;
}

void ProjectDialog::setupUI()
{
    setWindowTitle(m_isEditMode ? "Редактировать проект" : "Добавить проект");
    
    setStyleSheet(R"(
        QDialog { background-color: #2d2d2d; }
        QLabel { color: #cccccc; }
        QLineEdit, QPlainTextEdit {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            border-radius: 2px;
            padding: 4px;
        }
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 6px 20px;
            border-radius: 2px;
        }
        QPushButton:hover { background-color: #1177bb; }
    )");
}

void ProjectDialog::fillFromProject(ProjectPtr project)
{
    ui->editName->setText(project->getName());
    ui->editDescription->setPlainText(project->getDescription());
}

bool ProjectDialog::validate()
{
    if (ui->editName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название проекта не может быть пустым");
        return false;
    }
    return true;
}

void ProjectDialog::accept()
{
    if (validate()) {
        QDialog::accept();
    }
}

ProjectPtr ProjectDialog::getProject() const
{
    ProjectPtr project = m_isEditMode ? m_project : std::make_shared<Project>();
    project->setName(ui->editName->text().trimmed());
    project->setDescription(ui->editDescription->toPlainText().trimmed());
    return project;
}
