#include "AssignMachineDialog.h"
#include "ui_AssignMachineDialog.h"
#include "../database/FleetDatabase.h"
#include <QListWidget>
#include <QListWidgetItem>

AssignMachineDialog::AssignMachineDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AssignMachineDialog)
{
    ui->setupUi(this);
    setupUI();
}

AssignMachineDialog::~AssignMachineDialog()
{
    delete ui;
}

void AssignMachineDialog::setupUI()
{
    setWindowTitle("Назначить технику на проект");
    
    setStyleSheet(R"(
        QDialog { background-color: #2d2d2d; }
        QLabel { color: #cccccc; }
        QListWidget {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            border-radius: 2px;
        }
        QListWidget::item:selected {
            background-color: #094771;
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
    
    const auto projects = FleetDatabase::instance().getAllProjects();
    
    for (const auto& project : projects) {
        QListWidgetItem *item = new QListWidgetItem(project->getName());
        item->setData(Qt::UserRole, project->getId());
        ui->projectList->addItem(item);
    }
    
    if (!projects.isEmpty()) ui->projectList->setCurrentRow(0);
}

ProjectPtr AssignMachineDialog::getSelectedProject() const
{
    const int row = ui->projectList->currentRow();
    if (row < 0) return nullptr;
    
    const QListWidgetItem *item = ui->projectList->item(row);
    if (!item) return nullptr;
    
    const int projectId = item->data(Qt::UserRole).toInt();
    return FleetDatabase::instance().getProjectById(projectId);
}
