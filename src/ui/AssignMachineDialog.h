#pragma once

#include <QDialog>
#include "../models/Project.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AssignMachineDialog; }
QT_END_NAMESPACE

/**
 * @brief Диалог для назначения техники на проект
 */
class AssignMachineDialog : public QDialog {
    Q_OBJECT

public:
    explicit AssignMachineDialog(QWidget *parent = nullptr);
    ~AssignMachineDialog();
    
    ProjectPtr getSelectedProject() const;

private:
    void setupUI();
    
    Ui::AssignMachineDialog *ui;
};
