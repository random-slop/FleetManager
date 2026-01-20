#pragma once

#include <QDialog>
#include "../models/Project.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProjectDialog; }
QT_END_NAMESPACE

/**
 * @brief Диалог для добавления или редактирования проекта
 */
class ProjectDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProjectDialog(QWidget *parent = nullptr, const ProjectPtr& project = nullptr);
    ~ProjectDialog();
    
    ProjectPtr getProject() const;

protected:
    void accept() override;

private:
    void setupUI();
    void fillFromProject(ProjectPtr project);
    bool validate();
    
    Ui::ProjectDialog *ui;
    ProjectPtr m_project;
    bool m_isEditMode;
};
