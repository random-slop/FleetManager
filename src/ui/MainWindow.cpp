#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MachineTableModel.h"
#include "ProjectTableModel.h"
#include "MachineDialog.h"
#include "ProjectDialog.h"
#include "AssignMachineDialog.h"
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
#include <tuple>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_stackedWidget(nullptr)
    , m_tableModel(nullptr)
    , m_tableView(nullptr)
    , m_projectTableModel(nullptr)
    , m_projectTableView(nullptr)
    , m_statusFilter(nullptr)
    , m_btnFleet(nullptr)
    , m_btnProjects(nullptr)
{
    ui->setupUi(this);
    setupUI();
    connectSignals();
    
    // Загрузка данных
    m_tableModel->loadData();
    m_projectTableModel->refresh();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setupSidebar();
    
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
    
    setupDetailsPanel();
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

void MainWindow::setupSidebar()
{
    // Боковая панель с навигацией и фильтрами
    QVBoxLayout *sidebarLayout = new QVBoxLayout(ui->sidebarWidget);
    sidebarLayout->setContentsMargins(8, 8, 8, 8);
    sidebarLayout->setSpacing(8);
    
    // Заголовок навигации
    QLabel *navTitle = new QLabel("НАВИГАЦИЯ");
    navTitle->setStyleSheet("color: #858585; font-weight: bold; font-size: 11px; padding-bottom: 4px;");
    sidebarLayout->addWidget(navTitle);
    
    // Кнопки навигации
    m_btnFleet = new QPushButton("📋 Парк техники");
    m_btnFleet->setStyleSheet(R"(
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
    )");
    sidebarLayout->addWidget(m_btnFleet);
    
    m_btnProjects = new QPushButton("🏗️ Проекты");
    m_btnProjects->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: transparent;
            color: #cccccc;
            border: none;
        }
        QPushButton:hover {
            background-color: #2a2d2e;
        }
    )");
    sidebarLayout->addWidget(m_btnProjects);
    
    QPushButton *btnHistory = new QPushButton("📜 История");
    btnHistory->setStyleSheet(m_btnProjects->styleSheet());
    sidebarLayout->addWidget(btnHistory);
    
    QPushButton *btnReports = new QPushButton("📊 Отчёты");
    btnReports->setStyleSheet(m_btnProjects->styleSheet());
    sidebarLayout->addWidget(btnReports);
    
    QPushButton *btnSettings = new QPushButton("⚙️ Настройки");
    btnSettings->setStyleSheet(m_btnProjects->styleSheet());
    sidebarLayout->addWidget(btnSettings);
    
    sidebarLayout->addSpacing(20);
    
    // Фильтр по статусу
    QLabel *filterTitle = new QLabel("ФИЛЬТР");
    filterTitle->setStyleSheet("color: #858585; font-weight: bold; font-size: 11px; padding-bottom: 4px;");
    sidebarLayout->addWidget(filterTitle);
    
    m_statusFilter = new QComboBox();
    m_statusFilter->addItem("Все статусы");
    m_statusFilter->addItem("Свободна");
    m_statusFilter->addItem("На объекте");
    m_statusFilter->addItem("В ремонте");
    m_statusFilter->addItem("Списана");
    m_statusFilter->setStyleSheet(R"(
        QComboBox {
            padding: 6px;
            background-color: #3c3c3c;
            color: #cccccc;
            border: 1px solid #555555;
            border-radius: 2px;
        }
        QComboBox:hover {
            background-color: #4a4a4a;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #3c3c3c;
            color: #cccccc;
            selection-background-color: #094771;
        }
    )");
    sidebarLayout->addWidget(m_statusFilter);
    
    sidebarLayout->addStretch();
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
            border: none;
            border-bottom: 1px solid #3e3e3e;
            font-weight: bold;
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

void MainWindow::setupDetailsPanel()
{
    m_detailsPanel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_detailsPanel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    
    // Заголовок панели
    QLabel *title = new QLabel("ДЕТАЛИ ТЕХНИКИ");
    title->setStyleSheet("color: #858585; font-weight: bold; font-size: 11px; padding-bottom: 4px;");
    layout->addWidget(title);
    
    // Группа с информацией
    QGroupBox *infoGroup = new QGroupBox();
    infoGroup->setStyleSheet(R"(
        QGroupBox {
            background-color: #252526;
            border: 1px solid #3e3e3e;
            border-radius: 4px;
            padding: 12px;
        }
    )");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(8);
    
    // Создаём метки для отображения информации
    auto createInfoLabel = [](const QString& title, QLabel** valueLabel) -> QWidget* {
        QWidget *widget = new QWidget();
        QVBoxLayout *vbox = new QVBoxLayout(widget);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("color: #858585; font-size: 10px;");
        vbox->addWidget(titleLabel);
        
        *valueLabel = new QLabel("—");
        (*valueLabel)->setStyleSheet("color: #d4d4d4; font-size: 12px; font-weight: 500;");
        (*valueLabel)->setWordWrap(true);
        vbox->addWidget(*valueLabel);
        
        return widget;
    };
    
    infoLayout->addWidget(createInfoLabel("НАЗВАНИЕ", &m_detailsName));
    infoLayout->addWidget(createInfoLabel("ТИП ТЕХНИКИ", &m_detailsType));
    infoLayout->addWidget(createInfoLabel("СЕРИЙНЫЙ НОМЕР", &m_detailsSerial));
    infoLayout->addWidget(createInfoLabel("ГОД ВЫПУСКА", &m_detailsYear));
    infoLayout->addWidget(createInfoLabel("СТАТУС", &m_detailsStatus));
    infoLayout->addWidget(createInfoLabel("СТОИМОСТЬ", &m_detailsCost));
    infoLayout->addWidget(createInfoLabel("ТЕКУЩИЙ ПРОЕКТ", &m_detailsProject));
    infoLayout->addWidget(createInfoLabel("НАЗНАЧЕН С", &m_detailsAssignedDate));
    
    layout->addWidget(infoGroup);
    layout->addStretch();
    
    // Изначально показываем пустую панель
    updateDetailsPanel(nullptr);
}

void MainWindow::connectSignals()
{
    // Подключаем кнопки навигации
    connect(m_btnFleet, &QPushButton::clicked, this, &MainWindow::showFleetView);
    connect(m_btnProjects, &QPushButton::clicked, this, &MainWindow::showProjectsView);

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
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExit);
    
    // Подключаем выбор строки в таблице
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(m_projectTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onProjectSelectionChanged);
    
    // Подключаем фильтр по статусу
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onStatusFilterChanged);
}

void MainWindow::showFleetView()
{
    m_stackedWidget->setCurrentIndex(0);
    m_btnFleet->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: #094771;
            color: white;
            border: none;
            border-radius: 2px;
        }
    )");
    m_btnProjects->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: transparent;
            color: #cccccc;
            border: none;
        }
        QPushButton:hover { background-color: #2a2d2e; }
    )");
    m_statusFilter->setEnabled(true);
}

void MainWindow::showProjectsView()
{
    m_stackedWidget->setCurrentIndex(1);
    m_btnProjects->setStyleSheet(m_btnFleet->styleSheet());
    m_btnFleet->setStyleSheet(R"(
        QPushButton {
            text-align: left;
            padding: 8px 12px;
            background-color: transparent;
            color: #cccccc;
            border: none;
        }
        QPushButton:hover { background-color: #2a2d2e; }
    )");
    m_statusFilter->setEnabled(false);
}

void MainWindow::onAddMachine()
{
    MachineDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        const auto machine = dialog.getMachine();
        if (FleetDatabase::instance().addMachine(machine)) {
            m_tableModel->loadData();
            updateStatusBar();
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
    
    MachineDialog dialog(this, machine);
    if (dialog.exec() == QDialog::Accepted) {
        const auto updatedMachine = dialog.getMachine();
        if (FleetDatabase::instance().updateMachine(updatedMachine)) {
            m_tableModel->loadData();
            updateStatusBar();
            onTableSelectionChanged(); // Обновляем панель деталей
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
    
    const auto reply = QMessageBox::question(this, "Подтверждение",
                                      QString("Удалить проект \"%1\"?").arg(project->getName()),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (FleetDatabase::instance().deleteProject(project->getId())) {
            m_projectTableModel->refresh();
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
            m_tableModel->loadData();
            updateStatusBar();
            onTableSelectionChanged();
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
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Возврат с проекта", "Выберите технику");
        return;
    }
    
    if (machine->getStatus() != MachineStatus::OnSite) {
        QMessageBox::warning(this, "Возврат с проекта", 
                           "Можно вернуть только технику, которая находится на объекте");
        return;
    }
    
    // Обновляем статус
    machine->setStatus(MachineStatus::Available);
    machine->setCurrentProject("");
    machine->setAssignedDate(QDate());
    
    if (FleetDatabase::instance().updateMachine(machine)) {
        m_tableModel->loadData();
        updateStatusBar();
        onTableSelectionChanged(); // Обновляем панель деталей
        QMessageBox::information(this, "Возврат с проекта", 
                               QString("Техника \"%1\" возвращена в парк").arg(machine->getName()));
    } else QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
}

void MainWindow::onSendToRepair()
{
    const auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Отправка в ремонт", "Выберите технику");
        return;
    }
    
    if (machine->getStatus() == MachineStatus::InRepair) {
        QMessageBox::information(this, "Отправка в ремонт", "Техника уже в ремонте");
        return;
    }
    
    if (machine->getStatus() == MachineStatus::Decommissioned) {
        QMessageBox::warning(this, "Отправка в ремонт", "Списанную технику нельзя отправить в ремонт");
        return;
    }
    
    // Обновляем статус
    machine->setStatus(MachineStatus::InRepair);
    if (machine->getStatus() == MachineStatus::OnSite) {
        machine->setCurrentProject("");
        machine->setAssignedDate(QDate());
    }
    
    if (FleetDatabase::instance().updateMachine(machine)) {
        m_tableModel->loadData();
        updateStatusBar();
        onTableSelectionChanged();
        QMessageBox::information(this, "Отправка в ремонт", 
                               QString("Техника \"%1\" отправлена в ремонт").arg(machine->getName()));
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе",
                      "<h3>Парк техники</h3>"
                      "<p>Версия 1.0</p>"
                      "<p>Программа для управления парком строительной техники</p>"
                      "<p><b>Разработчик:</b> Вячеслав</p>"
                      "<p><b>Технологии:</b> Qt6, C++20, SQLite</p>"
                      "<p>Курсовая работа по дисциплине \"Программирование\"</p>");
}

void MainWindow::onExit()
{
    close();
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
    menu.addAction(ui->actionReturnFromProject);
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
    QMenu menu(this);
    for (int i = 0; i < m_tableModel->columnCount(); ++i) {
        QString title = m_tableModel->headerData(i, Qt::Horizontal).toString();
        QAction *action = menu.addAction(title);
        action->setCheckable(true);
        action->setChecked(!m_tableView->isColumnHidden(i));
        
        connect(action, &QAction::triggered, this, [this, i](bool checked){
            m_tableView->setColumnHidden(i, !checked);
        });
    }
    menu.exec(m_tableView->horizontalHeader()->mapToGlobal(pos));
}

void MainWindow::updateStatusBar() const
{
    const auto stats = FleetDatabase::instance().getStatistics();
    ui->statusbar->showMessage(QString("Всего: %1 | Свободно: %2 | На объектах: %3 | В ремонте: %4")
                              .arg(stats.total)
                              .arg(stats.available)
                              .arg(stats.onSite)
                              .arg(stats.inRepair));
}
