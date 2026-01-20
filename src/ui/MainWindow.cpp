#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MachineTableModel.h"
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
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(nullptr)
    , m_tableView(nullptr)
    , m_statusFilter(nullptr)
{
    ui->setupUi(this);
    setupUI();
    connectSignals();
    
    // Загрузка данных
    m_tableModel->loadData();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setupSidebar();
    setupTable();
    setupDetailsPanel();
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
    QPushButton *btnFleet = new QPushButton("📋 Парк техники");
    btnFleet->setStyleSheet(R"(
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
    sidebarLayout->addWidget(btnFleet);
    
    QPushButton *btnProjects = new QPushButton("🏗️ Проекты");
    btnProjects->setStyleSheet(R"(
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
    sidebarLayout->addWidget(btnProjects);
    
    QPushButton *btnHistory = new QPushButton("📜 История");
    btnHistory->setStyleSheet(btnProjects->styleSheet());
    sidebarLayout->addWidget(btnHistory);
    
    QPushButton *btnReports = new QPushButton("📊 Отчёты");
    btnReports->setStyleSheet(btnProjects->styleSheet());
    sidebarLayout->addWidget(btnReports);
    
    QPushButton *btnSettings = new QPushButton("⚙️ Настройки");
    btnSettings->setStyleSheet(btnProjects->styleSheet());
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
    m_tableView->setColumnWidth(0, 200); // Название
    m_tableView->setColumnWidth(1, 150); // Тип
    m_tableView->setColumnWidth(2, 150); // Серийный номер
    m_tableView->setColumnWidth(3, 100); // Год
    m_tableView->setColumnWidth(4, 120); // Статус
    m_tableView->setColumnWidth(5, 120); // Стоимость
    
    // Добавляем таблицу в контейнер
    QVBoxLayout *tableLayout = new QVBoxLayout(ui->tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->addWidget(m_tableView);
}

void MainWindow::setupDetailsPanel()
{
    m_detailsPanel = ui->detailsContainer;
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
    // Подключаем действия меню и toolbar
    connect(ui->actionAdd, &QAction::triggered, this, &MainWindow::onAddMachine);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::onEditMachine);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::onDeleteMachine);
    connect(ui->actionAssignToProject, &QAction::triggered, this, &MainWindow::onAssignToProject);
    connect(ui->actionReturnFromProject, &QAction::triggered, this, &MainWindow::onReturnFromProject);
    connect(ui->actionSendToRepair, &QAction::triggered, this, &MainWindow::onSendToRepair);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExit);
    
    // Подключаем выбор строки в таблице
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTableSelectionChanged);
    
    // Подключаем фильтр по статусу
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStatusFilterChanged);
}

void MainWindow::onAddMachine()
{
    // TODO: Создать диалог добавления техники
    QMessageBox::information(this, "Добавление", "Функция добавления техники будет реализована в следующей версии");
}

void MainWindow::onEditMachine()
{
    auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Редактирование", "Выберите технику для редактирования");
        return;
    }
    
    // TODO: Создать диалог редактирования
    QMessageBox::information(this, "Редактирование", 
                            QString("Редактирование: %1\nФункция будет реализована в следующей версии")
                            .arg(machine->getName()));
}

void MainWindow::onDeleteMachine()
{
    auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Удаление", "Выберите технику для удаления");
        return;
    }
    
    // Подтверждение удаления
    auto reply = QMessageBox::question(this, "Подтверждение удаления",
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

void MainWindow::onAssignToProject()
{
    auto machine = getSelectedMachine();
    if (!machine) {
        QMessageBox::warning(this, "Назначение на проект", "Выберите технику");
        return;
    }
    
    if (machine->getStatus() != MachineStatus::Available) {
        QMessageBox::warning(this, "Назначение на проект", 
                           "Можно назначать только свободную технику");
        return;
    }
    
    // TODO: Диалог выбора проекта
    QMessageBox::information(this, "Назначение на проект", 
                           "Функция назначения на проект будет реализована в следующей версии");
}

void MainWindow::onReturnFromProject()
{
    auto machine = getSelectedMachine();
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
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус техники");
    }
}

void MainWindow::onSendToRepair()
{
    auto machine = getSelectedMachine();
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

void MainWindow::onTableSelectionChanged()
{
    auto machine = getSelectedMachine();
    updateDetailsPanel(machine);
}

void MainWindow::onStatusFilterChanged(int index)
{
    m_tableModel->setStatusFilter(index);
    updateStatusBar();
}

void MainWindow::updateDetailsPanel(MachinePtr machine)
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
    
    m_detailsCost->setText(QString("%1 ₽").arg(machine->getCost(), 0, 'f', 0));
    m_detailsProject->setText(machine->getCurrentProject().isEmpty() ? "—" : machine->getCurrentProject());
    m_detailsAssignedDate->setText(machine->getAssignedDate().isValid() ?
                                  machine->getAssignedDate().toString("dd.MM.yyyy") : "—");
}

void MainWindow::updateStatusBar()
{
    auto stats = FleetDatabase::instance().getStatistics();
    QString statusText = QString("Всего техники: %1  |  Выбрано: %2 из %3")
                        .arg(stats.total)
                        .arg(1) // TODO: Получить реальное количество выбранных
                        .arg(24); // TODO: Получить реальное количество отфильтрованных
    
    // Добавляем статистику по статусам
    statusText += QString("  |  Свободна: %1  |  На объектах: %2  |  В ремонте: %3")
                 .arg(stats.available)
                 .arg(stats.onSite)
                 .arg(stats.inRepair);
    
    ui->statusbar->showMessage(statusText);
}

MachinePtr MainWindow::getSelectedMachine()
{
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        return nullptr;
    }
    
    int row = selection.first().row();
    return m_tableModel->getMachine(row);
}
