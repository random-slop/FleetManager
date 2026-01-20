#include "FleetDatabase.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

FleetDatabase& FleetDatabase::instance()
{
    static FleetDatabase instance;
    return instance;
}

FleetDatabase::FleetDatabase()
    : m_initialized(false)
{
}

FleetDatabase::~FleetDatabase()
{
    close();
}

bool FleetDatabase::initialize(const QString& dbPath)
{
    if (m_initialized) {
        return true;
    }
    
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qWarning() << "Не удалось открыть базу данных:" << m_database.lastError().text();
        return false;
    }
    
    if (!createTables()) {
        qWarning() << "Не удалось создать таблицы";
        return false;
    }
    
    // Создаём тестовые данные если база пустая
    QSqlQuery query("SELECT COUNT(*) FROM machines");
    if (query.next() && query.value(0).toInt() == 0) {
        createSampleData();
    }
    
    m_initialized = true;
    qDebug() << "База данных успешно инициализирована:" << dbPath;
    return true;
}

void FleetDatabase::close()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_initialized = false;
}

bool FleetDatabase::createTables()
{
    QSqlQuery query;
    
    // Создаём таблицу техники
    QString createMachinesTable = R"(
        CREATE TABLE IF NOT EXISTS machines (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            serial_number TEXT UNIQUE NOT NULL,
            year_of_manufacture INTEGER NOT NULL,
            status TEXT NOT NULL,
            cost REAL NOT NULL,
            current_project TEXT,
            assigned_date TEXT
        )
    )";
    
    if (!query.exec(createMachinesTable)) {
        qWarning() << "Ошибка создания таблицы machines:" << query.lastError().text();
        return false;
    }
    
    // Создаём таблицу проектов
    QString createProjectsTable = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            start_date TEXT NOT NULL,
            end_date TEXT,
            description TEXT
        )
    )";
    
    if (!query.exec(createProjectsTable)) {
        qWarning() << "Ошибка создания таблицы projects:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Таблицы успешно созданы";
    return true;
}

void FleetDatabase::createSampleData()
{
    qDebug() << "Создание тестовых данных...";
    
    // Добавляем проекты
    auto project1 = std::make_shared<Project>("ЖК «Солнечный»", QDate(2026, 1, 20));
    auto project2 = std::make_shared<Project>("БЦ «Меридиан»", QDate(2025, 11, 1));
    auto project3 = std::make_shared<Project>("Школа №15", QDate(2026, 1, 15));
    
    addProject(project1);
    addProject(project2);
    addProject(project3);
    
    // Добавляем технику
    auto machine1 = std::make_shared<Machine>("Экскаватор CAT 320D", "Экскаватор", "CAT320D-2019-0847", 2019, 8500000);
    machine1->setStatus(MachineStatus::OnSite);
    machine1->setCurrentProject("ЖК «Солнечный»");
    machine1->setAssignedDate(QDate(2026, 1, 20));
    addMachine(machine1);
    
    auto machine2 = std::make_shared<Machine>("Бульдозер Komatsu D65", "Бульдозер", "KOM-D65-2020-1123", 2020, 12300000);
    machine2->setStatus(MachineStatus::Available);
    addMachine(machine2);
    
    auto machine3 = std::make_shared<Machine>("Кран башенный КБ-403", "Кран", "KB403-2018-0291", 2018, 15700000);
    machine3->setStatus(MachineStatus::OnSite);
    machine3->setCurrentProject("БЦ «Меридиан»");
    addMachine(machine3);
    
    auto machine4 = std::make_shared<Machine>("Автокран Liebherr LTM 1050", "Автокран", "LTM1050-2021-0055", 2021, 22100000);
    machine4->setStatus(MachineStatus::InRepair);
    addMachine(machine4);
    
    auto machine5 = std::make_shared<Machine>("Погрузчик JCB 531-70", "Погрузчик", "JCB531-2019-0782", 2019, 4200000);
    machine5->setStatus(MachineStatus::Available);
    addMachine(machine5);
    
    auto machine6 = std::make_shared<Machine>("Экскаватор-погрузчик JCB 3CX", "Экскаватор-погрузчик", "JCB3CX-2020-0394", 2020, 5800000);
    machine6->setStatus(MachineStatus::OnSite);
    machine6->setCurrentProject("ЖК «Солнечный»");
    addMachine(machine6);
    
    auto machine7 = std::make_shared<Machine>("Самосвал КАМАЗ-6520", "Самосвал", "KMZ6520-2017-1847", 2017, 3900000);
    machine7->setStatus(MachineStatus::Available);
    addMachine(machine7);
    
    auto machine8 = std::make_shared<Machine>("Бетономешалка MAN TGS", "Бетономешалка", "MAN-TGS-2019-0621", 2019, 7200000);
    machine8->setStatus(MachineStatus::OnSite);
    machine8->setCurrentProject("Школа №15");
    addMachine(machine8);
    
    auto machine9 = std::make_shared<Machine>("Каток BOMAG BW 213", "Каток", "BOMAG213-2018-0183", 2018, 6100000);
    machine9->setStatus(MachineStatus::InRepair);
    addMachine(machine9);
    
    auto machine10 = std::make_shared<Machine>("Грейдер ДЗ-98", "Грейдер", "DZ98-2016-0095", 2016, 2800000);
    machine10->setStatus(MachineStatus::Decommissioned);
    addMachine(machine10);
    
    auto machine11 = std::make_shared<Machine>("Виброплита Wacker Neuson", "Виброплита", "WN-VP-2022-0012", 2022, 320000);
    machine11->setStatus(MachineStatus::Available);
    addMachine(machine11);
    
    auto machine12 = std::make_shared<Machine>("Компрессор Atlas Copco", "Компрессор", "AC-XAS-2021-0487", 2021, 890000);
    machine12->setStatus(MachineStatus::OnSite);
    machine12->setCurrentProject("БЦ «Меридиан»");
    addMachine(machine12);
    
    qDebug() << "Тестовые данные созданы";
}

// ===== ОПЕРАЦИИ С ТЕХНИКОЙ =====

bool FleetDatabase::addMachine(MachinePtr machine)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO machines (name, type, serial_number, year_of_manufacture, status, cost, current_project, assigned_date)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(machine->getName());
    query.addBindValue(machine->getType());
    query.addBindValue(machine->getSerialNumber());
    query.addBindValue(machine->getYearOfManufacture());
    query.addBindValue(Machine::statusToString(machine->getStatus()));
    query.addBindValue(machine->getCost());
    query.addBindValue(machine->getCurrentProject());
    query.addBindValue(machine->getAssignedDate().isValid() ? machine->getAssignedDate().toString(Qt::ISODate) : QVariant());
    
    if (!query.exec()) {
        qWarning() << "Ошибка добавления техники:" << query.lastError().text();
        return false;
    }
    
    machine->setId(query.lastInsertId().toInt());
    return true;
}

bool FleetDatabase::updateMachine(MachinePtr machine)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE machines 
        SET name = ?, type = ?, serial_number = ?, year_of_manufacture = ?, 
            status = ?, cost = ?, current_project = ?, assigned_date = ?
        WHERE id = ?
    )");
    
    query.addBindValue(machine->getName());
    query.addBindValue(machine->getType());
    query.addBindValue(machine->getSerialNumber());
    query.addBindValue(machine->getYearOfManufacture());
    query.addBindValue(Machine::statusToString(machine->getStatus()));
    query.addBindValue(machine->getCost());
    query.addBindValue(machine->getCurrentProject());
    query.addBindValue(machine->getAssignedDate().isValid() ? machine->getAssignedDate().toString(Qt::ISODate) : QVariant());
    query.addBindValue(machine->getId());
    
    if (!query.exec()) {
        qWarning() << "Ошибка обновления техники:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool FleetDatabase::deleteMachine(int machineId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM machines WHERE id = ?");
    query.addBindValue(machineId);
    
    if (!query.exec()) {
        qWarning() << "Ошибка удаления техники:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QVector<MachinePtr> FleetDatabase::getAllMachines()
{
    QVector<MachinePtr> machines;
    QSqlQuery query("SELECT * FROM machines ORDER BY id");
    
    while (query.next()) {
        auto machine = std::make_shared<Machine>();
        machine->setId(query.value("id").toInt());
        machine->setName(query.value("name").toString());
        machine->setType(query.value("type").toString());
        machine->setSerialNumber(query.value("serial_number").toString());
        machine->setYearOfManufacture(query.value("year_of_manufacture").toInt());
        machine->setStatus(Machine::stringToStatus(query.value("status").toString()));
        machine->setCost(query.value("cost").toDouble());
        machine->setCurrentProject(query.value("current_project").toString());
        
        QString dateStr = query.value("assigned_date").toString();
        if (!dateStr.isEmpty()) {
            machine->setAssignedDate(QDate::fromString(dateStr, Qt::ISODate));
        }
        
        machines.append(machine);
    }
    
    return machines;
}

MachinePtr FleetDatabase::getMachineById(int machineId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM machines WHERE id = ?");
    query.addBindValue(machineId);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    auto machine = std::make_shared<Machine>();
    machine->setId(query.value("id").toInt());
    machine->setName(query.value("name").toString());
    machine->setType(query.value("type").toString());
    machine->setSerialNumber(query.value("serial_number").toString());
    machine->setYearOfManufacture(query.value("year_of_manufacture").toInt());
    machine->setStatus(Machine::stringToStatus(query.value("status").toString()));
    machine->setCost(query.value("cost").toDouble());
    machine->setCurrentProject(query.value("current_project").toString());
    
    QString dateStr = query.value("assigned_date").toString();
    if (!dateStr.isEmpty()) {
        machine->setAssignedDate(QDate::fromString(dateStr, Qt::ISODate));
    }
    
    return machine;
}

QVector<MachinePtr> FleetDatabase::getMachinesByStatus(MachineStatus status)
{
    QVector<MachinePtr> machines;
    QSqlQuery query;
    query.prepare("SELECT * FROM machines WHERE status = ? ORDER BY id");
    query.addBindValue(Machine::statusToString(status));
    
    if (!query.exec()) {
        return machines;
    }
    
    while (query.next()) {
        auto machine = std::make_shared<Machine>();
        machine->setId(query.value("id").toInt());
        machine->setName(query.value("name").toString());
        machine->setType(query.value("type").toString());
        machine->setSerialNumber(query.value("serial_number").toString());
        machine->setYearOfManufacture(query.value("year_of_manufacture").toInt());
        machine->setStatus(Machine::stringToStatus(query.value("status").toString()));
        machine->setCost(query.value("cost").toDouble());
        machine->setCurrentProject(query.value("current_project").toString());
        
        QString dateStr = query.value("assigned_date").toString();
        if (!dateStr.isEmpty()) {
            machine->setAssignedDate(QDate::fromString(dateStr, Qt::ISODate));
        }
        
        machines.append(machine);
    }
    
    return machines;
}

// ===== ОПЕРАЦИИ С ПРОЕКТАМИ =====

bool FleetDatabase::addProject(ProjectPtr project)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO projects (name, start_date, end_date, description)
        VALUES (?, ?, ?, ?)
    )");
    
    query.addBindValue(project->getName());
    query.addBindValue(project->getStartDate().toString(Qt::ISODate));
    query.addBindValue(project->getEndDate().isValid() ? project->getEndDate().toString(Qt::ISODate) : QVariant());
    query.addBindValue(project->getDescription());
    
    if (!query.exec()) {
        qWarning() << "Ошибка добавления проекта:" << query.lastError().text();
        return false;
    }
    
    project->setId(query.lastInsertId().toInt());
    return true;
}

bool FleetDatabase::updateProject(ProjectPtr project)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE projects 
        SET name = ?, start_date = ?, end_date = ?, description = ?
        WHERE id = ?
    )");
    
    query.addBindValue(project->getName());
    query.addBindValue(project->getStartDate().toString(Qt::ISODate));
    query.addBindValue(project->getEndDate().isValid() ? project->getEndDate().toString(Qt::ISODate) : QVariant());
    query.addBindValue(project->getDescription());
    query.addBindValue(project->getId());
    
    if (!query.exec()) {
        qWarning() << "Ошибка обновления проекта:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool FleetDatabase::deleteProject(int projectId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM projects WHERE id = ?");
    query.addBindValue(projectId);
    
    if (!query.exec()) {
        qWarning() << "Ошибка удаления проекта:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QVector<ProjectPtr> FleetDatabase::getAllProjects()
{
    QVector<ProjectPtr> projects;
    QSqlQuery query("SELECT * FROM projects ORDER BY id");
    
    while (query.next()) {
        auto project = std::make_shared<Project>();
        project->setId(query.value("id").toInt());
        project->setName(query.value("name").toString());
        project->setStartDate(QDate::fromString(query.value("start_date").toString(), Qt::ISODate));
        
        QString endDateStr = query.value("end_date").toString();
        if (!endDateStr.isEmpty()) {
            project->setEndDate(QDate::fromString(endDateStr, Qt::ISODate));
        }
        
        project->setDescription(query.value("description").toString());
        projects.append(project);
    }
    
    return projects;
}

ProjectPtr FleetDatabase::getProjectById(int projectId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM projects WHERE id = ?");
    query.addBindValue(projectId);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    auto project = std::make_shared<Project>();
    project->setId(query.value("id").toInt());
    project->setName(query.value("name").toString());
    project->setStartDate(QDate::fromString(query.value("start_date").toString(), Qt::ISODate));
    
    QString endDateStr = query.value("end_date").toString();
    if (!endDateStr.isEmpty()) {
        project->setEndDate(QDate::fromString(endDateStr, Qt::ISODate));
    }
    
    project->setDescription(query.value("description").toString());
    return project;
}

// ===== СТАТИСТИКА =====

FleetDatabase::Statistics FleetDatabase::getStatistics()
{
    Statistics stats{0, 0, 0, 0, 0};
    
    QSqlQuery query("SELECT status, COUNT(*) as count FROM machines GROUP BY status");
    while (query.next()) {
        QString status = query.value("status").toString();
        int count = query.value("count").toInt();
        
        if (status == "Свободна") stats.available = count;
        else if (status == "На объекте") stats.onSite = count;
        else if (status == "В ремонте") stats.inRepair = count;
        else if (status == "Списана") stats.decommissioned = count;
    }
    
    stats.total = stats.available + stats.onSite + stats.inRepair + stats.decommissioned;
    return stats;
}
