#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MachineTableModel.h"
#include "ProjectTableModel.h"
#include "MachineDialog.h"
#include "ProjectDialog.h"
#include "AssignMachineDialog.h"
#include "SettingsDialog.h"
#include "../database/FleetDatabase.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QDebug>
#include <QStackedWidget>
#include <QSplitter>
#include <QSignalBlocker>
#include <tuple>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_stackedWidget(nullptr)
    , m_tableModel(nullptr)
    , m_tableView(nullptr)
    , m_projectTableModel(nullptr)
    , m_projectTableView(nullptr)
{
    ui->setupUi(this);
    
    setupUI();
    connectSignals();
    
    // Загрузка данных
    m_tableModel->loadData();
    m_projectTableModel->refresh();
    updateStatusBar();
    updateToolbarButtonsState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // Disable context menu on all toolbar buttons
    for (QObject *obj : ui->toolBar->children())
        if (const auto widget = qobject_cast<QWidget*>(obj))
            widget->setContextMenuPolicy(Qt::NoContextMenu);
    
    // Создаем StackedWidget для переключения между таблицами
    m_stackedWidget = new QStackedWidget();
    
    // Контейнер для техники (таблица + детали)
    QWidget *fleetView = new QWidget();
    QHBoxLayout *fleetLayout = new QHBoxLayout(fleetView);
    fleetLayout->setContentsMargins(0, 0, 0, 0);
    fleetLayout->setSpacing(0);
    
    QSplitter *fleetSplitter = new QSplitter(Qt::Horizontal);
    
    QWidget *tableContainer = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    
    setupTable();
    tableLayout->addWidget(m_tableView);
    
    fleetSplitter->addWidget(tableContainer);
    
    m_detailsPanel = ui->detailsPanel;
    m_detailsName = ui->detailsName;
    m_detailsType = ui->detailsType;
    m_detailsSerial = ui->detailsSerial;
    m_detailsYear = ui->detailsYear;
    m_detailsStatus = ui->detailsStatus;
    m_detailsCost = ui->detailsCost;
    m_detailsProject = ui->detailsProject;
    m_detailsAssignedDate = ui->detailsAssignedDate;
    fleetSplitter->addWidget(m_detailsPanel);
    
    fleetLayout->addWidget(fleetSplitter);
    
    // Контейнер для проектов
    QWidget *projectsView = new QWidget();
    QVBoxLayout *projectsLayout = new QVBoxLayout(projectsView);
    projectsLayout->setContentsMargins(0, 0, 0, 0);
    
    setupProjectsTable();
    projectsLayout->addWidget(m_projectTableView);
    
    m_stackedWidget->addWidget(fleetView);
    m_stackedWidget->addWidget(projectsView);
    
    // Заменяем старый splitter в UI на наш stackedWidget
    QHBoxLayout *hLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (hLayout) {
        // Удаляем splitter
        if (ui->splitter) {
            delete ui->splitter;
        }
        hLayout->addWidget(m_stackedWidget);
    }
}

void MainWindow::setupTable()
{
    // Создаём модель таблицы
    m_tableModel = new MachineTableModel(this);
    
    // Создаём представление таблицы
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setStyleSheet(R"(
        QTableView {
            background-color: #1e1e1e;
            color: #d4d4d4;
            gridline-color: #2d2d2d;
            border: none;
            selection-background-color: #264f78;
        }
        QTableView::item:selected {
            background-color: #264f78;
        }
        QTableView::item:hover {
            background-color: #2a2d2e;
        }
        QHeaderView::section {
            background-color: #2d2d2d;
            color: #cccccc;
            padding: 6px;
            border: 1px solid #1a1a1a;
            font-weight: bold;
        }
        QTableView QTableCornerButton::section {
            background-color: #2d2d2d;
            border: none;
        }
    )");
    
    // Устанавливаем ширину колонок
    m_tableView->setColumnWidth(0, 250); // Название
    m_tableView->setColumnWidth(1, 120); // Статус
    m_tableView->setColumnWidth(2, 200); // Текущий проект
    m_tableView->setColumnWidth(3, 150); // Тип техники
    m_tableView->setColumnWidth(4, 150); // Серийный номер
    m_tableView->setColumnWidth(5, 120); // Год выпуска
    m_tableView->setColumnWidth(6, 150); // Стоимость
    m_tableView->setColumnWidth(7, 120); // Назначен с
    
    // Включаем сортировку по колонкам
    m_tableView->setSortingEnabled(true);
    
    // Двойной клик для редактирования
    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onEditMachine);
    
    // Контекстное меню (ПКМ) на строках таблицы
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
    
    // Контекстное меню (ПКМ) на заголовке таблицы
    m_tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &MainWindow::showColumnHeaderMenu);


}

void MainWindow::setupProjectsTable()
{
    m_projectTableModel = new ProjectTableModel(this);
    m_projectTableView = new QTableView();
    m_projectTableView->setModel(m_projectTableModel);
    m_projectTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_projectTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_projectTableView->setAlternatingRowColors(true);
    m_projectTableView->verticalHeader()->setVisible(false);
    m_projectTableView->horizontalHeader()->setStretchLastSection(true);
    m_projectTableView->setStyleSheet(m_tableView->styleSheet());
    
    m_projectTableView->setColumnWidth(0, 50);  // ID
    m_projectTableView->setColumnWidth(1, 300); // Название
    m_projectTableView->setColumnWidth(2, 150); // Дата начала
    m_projectTableView->setColumnWidth(3, 150); // Дата окончания
    
    m_projectTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_projectTableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showProjectContextMenu);
    
    connect(m_projectTableView, &QTableView::doubleClicked, this, &MainWindow::onEditProject);
}

void MainWindow::connectSignals()
{
    // Подключаем кнопки навигации
    connect(ui->btnFleet, &QPushButton::clicked, this, &MainWindow::showFleetView);
    connect(ui->btnProjects, &QPushButton::clicked, this, &MainWindow::showProjectsView);
    connect(ui->btnSettings, &QPushButton::clicked, this, &MainWindow::onShowSettings);

    // Подключаем действия меню и toolbar
    connect(ui->actionAdd, &QAction::triggered, this, [this](){
        if (m_stackedWidget->currentIndex() == 0) onAddMachine();
        else onAddProject();
    });
    connect(ui->actionEdit, &QAction::triggered, this, [this](){
        if (m_stackedWidget->currentIndex() == 0) onEditMachine();
        else onEditProject();
    });
    connect(ui->actionDelete, &QAction::triggered, this, [this](){
        if (m_stackedWidget->currentIndex() == 0) onDeleteMachine();
        else onDeleteProject();
    });

    connect(ui->actionAssignToProject, &QAction::triggered, this, &MainWindow::onAssignToProject);
    connect(ui->actionReturnFromProject, &QAction::triggered, this, &MainWindow::onReturnFromProject);
    connect(ui->actionSendToRepair, &QAction::triggered, this, &MainWindow::onSendToRepair);
    
    // Подключаем выбор строки в таблице
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(m_projectTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onProjectSelectionChanged);
    
    // Подключаем обновление состояния toolbar кнопок
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateToolbarButtonsState);
    connect(m_projectTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateToolbarButtonsState);
    
    // Подключаем фильтр по статусу
    connect(ui->statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onStatusFilterChanged);

}

void MainWindow::showFleetView()
{
    m_stackedWidget->setCurrentIndex(0);
    
    // Swap styles between buttons - btnFleet becomes active (blue), btnProjects becomes inactive
    QString activeStyle = R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: #094771;
            color: white;
            border: none;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #0e639c;
        }
    )";
    
    QString inactiveStyle = R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: transparent;
            color: #cccccc;
            border: none;
        }
        QPushButton:hover { background-color: #2a2d2e; }
    )";
    
    ui->btnFleet->setStyleSheet(activeStyle);
    ui->btnProjects->setStyleSheet(inactiveStyle);
    ui->statusFilter->setEnabled(true);
    updateStatusBar();
    updateToolbarButtonsState();
}

void MainWindow::showProjectsView()
{
    m_stackedWidget->setCurrentIndex(1);
    
    // Swap styles - btnProjects becomes active (blue), btnFleet becomes inactive
    QString activeStyle = R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: #094771;
            color: white;
            border: none;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #0e639c;
        }
    )";
    
    QString inactiveStyle = R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: transparent;
            color: #cccccc;
            border: none;
        }
        QPushButton:hover { background-color: #2a2d2e; }
    )";
    
    ui->btnProjects->setStyleSheet(activeStyle);
    ui->btnFleet->setStyleSheet(inactiveStyle);
    ui->statusFilter->setEnabled(false);
    updateStatusBar();
    updateToolbarButtonsState();
}

void MainWindow::onAddMachine()
{
    MachineDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const auto machine = dialog.getMachine();
        if (FleetDatabase::instance().addMachine(machine)) {
            m_tableModel->loadData();
            updateStatusBar();
            updateToolbarButtonsState();
            QMessageBox::information(this, "Добавление",
                                   QString("Техника \"%1\" успешно добавлена").arg(machine->getName()));
        } else QMessageBox::critical(this, "Ошибка", "Не удалось добавить технику в базу данных");
    }
}

void MainWindow::onEditMachine()
{
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Редактирование", "Выберите технику для редактирования");
        return;
    }
    
    int selectedMachineId = machine->getId();
    
    MachineDialog dialog(this, machine);
    if (dialog.exec() == QDialog::Accepted) {
        const auto updatedMachine = dialog.getMachine();
        if (FleetDatabase::instance().updateMachine(updatedMachine)) {
            // Временно отключаем сигналы от выбора для предотвращения обновления panels during reload
            const QSignalBlocker blocker(m_tableView->selectionModel());
            
            m_tableModel->loadData();
            updateStatusBar();
            restoreMachineSelection(selectedMachineId); // Восстанавливаем выделение
            updateToolbarButtonsState();
            QMessageBox::information(this, "Редактирование",
                                   QString("Техника \"%1\" успешно обновлена").arg(updatedMachine->getName()));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить технику в базе данных");
        }
    }
}

void MainWindow::onDeleteMachine()
{
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Удаление", "Выберите технику для удаления");
        return;
    }
    
    // Подтверждение удаления
    const auto reply = QMessageBox::question(this, "Подтверждение удаления",
                                      QString("Удалить технику \"%1\"?\nЭто действие нельзя отменить.")
                                      .arg(machine->getName()),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (FleetDatabase::instance().deleteMachine(machine->getId())) {
            m_tableModel->loadData();
            updateStatusBar();
            updateDetailsPanel(nullptr);
            updateToolbarButtonsState();
            QMessageBox::information(this, "Удаление", "Техника успешно удалена");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить технику");
        }
    }
}

void MainWindow::onAddProject()
{
    ProjectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const auto project = dialog.getProject();
        if (FleetDatabase::instance().addProject(project)) {
            m_projectTableModel->refresh();
            updateToolbarButtonsState();
            QMessageBox::information(this, "Добавление",
                                   QString("Проект \"%1\" успешно добавлен").arg(project->getName()));
        } else QMessageBox::critical(this, "Ошибка", "Не удалось добавить проект");
    }
}

void MainWindow::onEditProject()
{
    const auto project = getSelectedProject();
    if (!project) {
        QMessageBox::warning(this, "Редактирование", "Выберите проект для редактирования");
        return;
    }
    
    ProjectDialog dialog(this, project);
    if (dialog.exec() == QDialog::Accepted) {
        const auto updatedProject = dialog.getProject();
        if (FleetDatabase::instance().updateProject(updatedProject)) {
            m_projectTableModel->refresh();
            updateToolbarButtonsState();
            QMessageBox::information(this, "Редактирование",
                                   QString("Проект \"%1\" успешно обновлен").arg(updatedProject->getName()));
        } else QMessageBox::critical(this, "Ошибка", "Не удалось обновить проект");
    }
}

void MainWindow::onDeleteProject()
{
    const auto project = getSelectedProject();
    if (!project) {
        QMessageBox::warning(this, "Удаление", "Выберите проект для удаления");
        return;
    }
    
    // Проверяем, есть ли машины на этом проекте
    auto machinesOnProject = FleetDatabase::instance().getMachinesByProject(project->getName());
    
    if (!machinesOnProject.isEmpty()) {
        QMessageBox::warning(this, "Удаление невозможно",
            QString("Невозможно удалить проект \"%1\".\n"
                    "На проекте работают %2 единиц техники.\n"
                    "Сначала верните всю технику с проекта.")
            .arg(project->getName())
            .arg(machinesOnProject.size()));
        return;
    }
    
    const auto reply = QMessageBox::question(this, "Подтверждение",
                                      QString("Удалить проект \"%1\"?").arg(project->getName()),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (FleetDatabase::instance().deleteProject(project->getId())) {
            m_projectTableModel->refresh();
            updateToolbarButtonsState();
            QMessageBox::information(this, "Удаление", "Проект успешно удален");
        } else QMessageBox::critical(this, "Ошибка", "Не удалось удалить проект");
    }
}

void MainWindow::onAssignToProject()
{
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Назначение на проект", "Выберите технику");
        return;
    }
    
    int selectedMachineId = machine->getId();
    
    // Если машина на объекте - вернуть с проекта
    if (machine->getStatus() == MachineStatus::OnSite) {
        machine->setStatus(MachineStatus::Available);
        machine->setCurrentProject("");
        machine->setAssignedDate(QDate());
        
        if (FleetDatabase::instance().updateMachine(machine)) {
            const QSignalBlocker blocker(m_tableView->selectionModel());
            m_tableModel->loadData();
            updateStatusBar();
            restoreMachineSelection(selectedMachineId);
            updateToolbarButtonsState();
            QMessageBox::information(this, "Возврат с проекта",
                                   QString("Техника \"%1\" возвращена в парк").arg(machine->getName()));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
        }
        return;
    }
    
    // Если машина свободна - назначить на проект
    if (machine->getStatus() != MachineStatus::Available) {
        QMessageBox::warning(this, "Назначение на проект",
                           "Можно назначать только свободную технику");
        return;
    }
    
    AssignMachineDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const auto project = dialog.getSelectedProject();
        if (!project) {
            QMessageBox::warning(this, "Назначение на проект", "Выберите проект");
            return;
        }
        
        machine->setStatus(MachineStatus::OnSite);
        machine->setCurrentProject(project->getName());
        machine->setAssignedDate(QDate::currentDate());
        
        if (FleetDatabase::instance().updateMachine(machine)) {
            const QSignalBlocker blocker(m_tableView->selectionModel());
            m_tableModel->loadData();
            updateStatusBar();
            restoreMachineSelection(selectedMachineId);
            updateToolbarButtonsState();
            QMessageBox::information(this, "Назначение на проект",
                                   QString("Техника \"%1\" назначена на проект \"%2\"")
                                   .arg(machine->getName(), project->getName()));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось назначить технику на проект");
        }
    }
}

void MainWindow::onReturnFromProject()
{
    // Эта функция больше не используется - функционал объединен с onAssignToProject()
    // Вызываем onAssignToProject() для совместимости
    onAssignToProject();
}

void MainWindow::onSendToRepair()
{
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Операция с ремонтом", "Выберите технику");
        return;
    }
    
    int selectedMachineId = machine->getId();
    
    if (machine->getStatus() == MachineStatus::Decommissioned) {
        QMessageBox::warning(this, "Операция с ремонтом", "Списанную технику нельзя отправить в ремонт");
        return;
    }
    
    // Если машина в ремонте - вернуть из ремонта
    if (machine->getStatus() == MachineStatus::InRepair) {
        machine->setStatus(MachineStatus::Available);
        
        if (FleetDatabase::instance().updateMachine(machine)) {
            const QSignalBlocker blocker(m_tableView->selectionModel());
            m_tableModel->loadData();
            updateStatusBar();
            restoreMachineSelection(selectedMachineId);
            updateToolbarButtonsState();
            QMessageBox::information(this, "Возврат из ремонта",
                                   QString("Техника \"%1\" возвращена из ремонта").arg(machine->getName()));
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
        }
        return;
    }
    
    // Отправить машину в ремонт
    MachineStatus oldStatus = machine->getStatus();
    machine->setStatus(MachineStatus::InRepair);
    
    // Если машина была на объекте - снять её с проекта
    if (oldStatus == MachineStatus::OnSite) {
        machine->setCurrentProject("");
        machine->setAssignedDate(QDate());
    }
    
    if (FleetDatabase::instance().updateMachine(machine)) {
        const QSignalBlocker blocker(m_tableView->selectionModel());
        m_tableModel->loadData();
        updateStatusBar();
        restoreMachineSelection(selectedMachineId);
        updateToolbarButtonsState();
        QMessageBox::information(this, "Отправка в ремонт",
                               QString("Техника \"%1\" отправлена в ремонт").arg(machine->getName()));
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
    }
}

void MainWindow::onTableSelectionChanged() const
{
    const auto machine = getSelectedMachine();
    updateDetailsPanel(machine);
}

void MainWindow::onProjectSelectionChanged() const
{
    // Можно добавить панель деталей для проектов, если нужно
}

void MainWindow::onStatusFilterChanged(int index) const
{
    m_tableModel->setStatusFilter(index);
    updateStatusBar();
}

void MainWindow::updateDetailsPanel(const MachinePtr& machine) const
{
    if (!machine) {
        m_detailsName->setText("—");
        m_detailsType->setText("—");
        m_detailsSerial->setText("—");
        m_detailsYear->setText("—");
        m_detailsStatus->setText("—");
        m_detailsCost->setText("—");
        m_detailsProject->setText("—");
        m_detailsAssignedDate->setText("—");
        return;
    }
    
    m_detailsName->setText(machine->getName());
    m_detailsType->setText(machine->getType());
    m_detailsSerial->setText(machine->getSerialNumber());
    m_detailsYear->setText(QString::number(machine->getYearOfManufacture()));
    
    // Статус с цветовым кодированием
    QString statusText = Machine::statusToString(machine->getStatus());
    QString statusColor;
    switch (machine->getStatus()) {
        case MachineStatus::Available:
            statusColor = "#4CAF50"; break;
        case MachineStatus::OnSite:
            statusColor = "#2196F3"; break;
        case MachineStatus::InRepair:
            statusColor = "#FF9800"; break;
        case MachineStatus::Decommissioned:
            statusColor = "#F44336"; break;
    }
    m_detailsStatus->setText(QString("<span style='color: %1; font-weight: bold;'>%2</span>")
                            .arg(statusColor, statusText));
    
    // Стоимость в оригинальной валюте и в рублях в скобках
    Money cost = machine->getCost();
    QString costText = cost.toString();
    if (cost.getCurrency() != Currency::RUB) {
        double rubAmount = cost.getAmount() * FleetDatabase::instance().getCurrencyRate(Money::getCurrencyName(cost.getCurrency()), "RUB");
        costText += QString(" (%1)").arg(Money(rubAmount, Currency::RUB).toString());
    }
    m_detailsCost->setText(costText);
    
    m_detailsProject->setText(machine->getCurrentProject().isEmpty() ? "—" : machine->getCurrentProject());
    m_detailsAssignedDate->setText(machine->getAssignedDate().isValid() ? machine->getAssignedDate().toString("dd.MM.yyyy") : "—");
}

MachinePtr MainWindow::getSelectedMachine() const
{
    const QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return nullptr;
    return m_tableModel->getMachine(selection.first().row());
}

ProjectPtr MainWindow::getSelectedProject() const
{
    const QModelIndexList selection = m_projectTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return nullptr;
    return m_projectTableModel->getProject(selection.first().row());
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_tableView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    menu.addAction(ui->actionEdit);
    menu.addAction(ui->actionDelete);
    menu.addSeparator();
    menu.addAction(ui->actionAssignToProject);
    menu.addSeparator();
    menu.addAction(ui->actionSendToRepair);
    
    menu.exec(m_tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::showProjectContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_projectTableView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);
    QAction *editAction = menu.addAction("Редактировать проект");
    QAction *deleteAction = menu.addAction("Удалить проект");
    
    connect(editAction, &QAction::triggered, this, &MainWindow::onEditProject);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteProject);
    
    menu.exec(m_projectTableView->viewport()->mapToGlobal(pos));
}

void MainWindow::showColumnHeaderMenu(const QPoint& pos)
{
    QMenu menu;
    
    // Получить информацию о всех колонках
    auto columnsInfo = m_tableModel->getColumnsInfo();
    
    // Создать чекбокс-действия для каждого столбца
    QVector<QAction*> columnActions;
    for (const auto& info : columnsInfo) {
        int columnIndex = std::get<0>(info);
        QString columnName = std::get<1>(info);
        bool isVisible = std::get<2>(info);
        
        QAction *action = menu.addAction(columnName);
        action->setCheckable(true);
        action->setChecked(isVisible);
        action->setData(columnIndex);
        columnActions.append(action);
    }
    
    // Показать меню
    QAction *selectedAction = menu.exec(m_tableView->horizontalHeader()->mapToGlobal(pos));
    
    // Обработать выбранное действие
    if (selectedAction) {
        int columnIndex = selectedAction->data().toInt();
        bool newVisibility = selectedAction->isChecked();
        m_tableModel->setColumnVisible(columnIndex, newVisibility);
    }
}

void MainWindow::updateStatusBar() const
{
    if (m_stackedWidget->currentIndex() == 0) {
        // Fleet view - show machine statistics
        const auto stats = FleetDatabase::instance().getStatistics();
        ui->statusbar->showMessage(QString("Всего: %1 | Свободно: %2 | На объектах: %3 | В ремонте: %4 | Списано: %5")
                                   .arg(stats.total)
                                   .arg(stats.available)
                                   .arg(stats.onSite)
                                   .arg(stats.inRepair)
                                   .arg(stats.decommissioned));
    } else if (m_stackedWidget->currentIndex() == 1) {
        // Projects view - show project statistics
        const auto allProjects = FleetDatabase::instance().getAllProjects();
        int totalProjects = allProjects.size();
        int activeProjects = 0;
        
        for (const auto& project : allProjects) {
            auto machines = FleetDatabase::instance().getMachinesByProject(project->getName());
            if (!machines.isEmpty()) {
                activeProjects++;
            }
        }
        
        ui->statusbar->showMessage(QString("Всего проектов: %1 | С техникой: %2")
                                   .arg(totalProjects)
                                   .arg(activeProjects));
    }
}

void MainWindow::updateToolbarButtonsState()
{
    bool isFleetView = (m_stackedWidget->currentIndex() == 0);
    bool isProjectsView = (m_stackedWidget->currentIndex() == 1);
    
    if (isFleetView) {
        const auto machine = getSelectedMachine();
        bool hasMachineSelected = (machine != nullptr);
        
        // actionEdit, actionDelete доступны только если что-то выбрано
        ui->actionEdit->setEnabled(hasMachineSelected);
        ui->actionDelete->setEnabled(hasMachineSelected);
        
        // actionAssignToProject: техника выбрана И (свободна ИЛИ на объекте)
        // Кнопка будет переключаться для назначения на проект или снятия с него
        bool canToggleAssignment = hasMachineSelected &&
                                  (machine->getStatus() == MachineStatus::Available ||
                                   machine->getStatus() == MachineStatus::OnSite);
        ui->actionAssignToProject->setEnabled(canToggleAssignment);
        
        // actionReturnFromProject больше не используется (объединена с actionAssignToProject)
        ui->actionReturnFromProject->setEnabled(false);
        
        // actionSendToRepair: техника выбрана И не списана
        // Кнопка будет переключаться для отправки в ремонт или возврата из ремонта
        bool canToggleRepair = hasMachineSelected &&
                              (machine->getStatus() != MachineStatus::Decommissioned);
        ui->actionSendToRepair->setEnabled(canToggleRepair);
    } else if (isProjectsView) {
        const auto project = getSelectedProject();
        bool hasProjectSelected = (project != nullptr);
        
        // actionEdit, actionDelete доступны только если что-то выбрано
        ui->actionEdit->setEnabled(hasProjectSelected);
        ui->actionDelete->setEnabled(hasProjectSelected);
        
        // Для проектов эти кнопки не используются
        ui->actionAssignToProject->setEnabled(false);
        ui->actionReturnFromProject->setEnabled(false);
        ui->actionSendToRepair->setEnabled(false);
    }
    
    updateActionTexts();
}

int MainWindow::saveSelectedMachineId() const
{
    const auto machine = getSelectedMachine();
    return machine ? machine->getId() : -1;
}

void MainWindow::restoreMachineSelection(int machineId)
{
    if (machineId <= 0) return;
    
    // Ищем машину с заданным ID в модели
    for (int row = 0; row < m_tableModel->rowCount(); ++row) {
        const auto machine = m_tableModel->getMachine(row);
        if (machine && machine->getId() == machineId) {
            // Выбираем эту строку
            QModelIndex index = m_tableModel->index(row, 0);
            m_tableView->setCurrentIndex(index);
            m_tableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            return;
        }
    }
}

void MainWindow::updateActionTexts()
{
    if (m_stackedWidget->currentIndex() == 0) {
        const auto machine = getSelectedMachine();
        
        // Обновляем текст кнопки назначения/снятия с проекта
        if (machine && machine->getStatus() == MachineStatus::OnSite) {
            ui->actionAssignToProject->setText("Вернуть с проекта");
            ui->actionAssignToProject->setToolTip("Вернуть технику с проекта");
        } else {
            ui->actionAssignToProject->setText("Назначить на проект");
            ui->actionAssignToProject->setToolTip("Назначить технику на проект");
        }
        
        // Обновляем текст кнопки отправки в ремонт/возврата из ремонта
        if (machine && machine->getStatus() == MachineStatus::InRepair) {
            ui->actionSendToRepair->setText("Вернуть из ремонта");
            ui->actionSendToRepair->setToolTip("Вернуть технику из ремонта");
        } else {
            ui->actionSendToRepair->setText("В ремонт");
            ui->actionSendToRepair->setToolTip("Отправить технику в ремонт");
        }
    }
}

void MainWindow::onShowSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}
