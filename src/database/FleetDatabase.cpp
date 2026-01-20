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

FleetDatabase::FleetDatabase(): m_initialized(false) {}

FleetDatabase::~FleetDatabase()
{
    close();
}

bool FleetDatabase::initialize(const QString& dbPath)
{
    if (m_initialized) return true;

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
    if (query.next() && query.value(0).toInt() == 0)
        createSampleData();

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
    const QString createMachinesTable = R"(
        CREATE TABLE IF NOT EXISTS machines (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            serial_number TEXT UNIQUE NOT NULL,
            year_of_manufacture INTEGER NOT NULL,
            status TEXT NOT NULL,
            cost REAL NOT NULL,
            currency TEXT NOT NULL DEFAULT 'RUB',
            current_project TEXT,
            assigned_date TEXT
        )
    )";
    
    if (!query.exec(createMachinesTable)) {
        qWarning() << "Ошибка создания таблицы machines:" << query.lastError().text();
        return false;
    }
    
    // Создаём таблицу проектов
    const QString createProjectsTable = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT
        )
    )";
    
    if (!query.exec(createProjectsTable)) {
        qWarning() << "Ошибка создания таблицы projects:" << query.lastError().text();
        return false;
    }
    
    // Создаём таблицу курсов валют
    const QString createCurrencyRatesTable = R"(
        CREATE TABLE IF NOT EXISTS currency_rates (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            from_currency TEXT NOT NULL,
            to_currency TEXT NOT NULL,
            rate REAL NOT NULL,
            UNIQUE(from_currency, to_currency)
        )
    )";
    
    if (!query.exec(createCurrencyRatesTable)) {
        qWarning() << "Ошибка создания таблицы currency_rates:" << query.lastError().text();
        return false;
    }
    
    // Инициализируем курсы валют по умолчанию
    initializeDefaultCurrencyRates();
    
    qDebug() << "Таблицы успешно созданы";
    return true;
}

void FleetDatabase::initializeDefaultCurrencyRates()
{
    // Проверяем, есть ли уже курсы в базе
    QSqlQuery checkQuery("SELECT COUNT(*) FROM currency_rates");
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        // Курсы уже есть, загружаем их в память
        loadCurrencyRates();
        return;
    }
    
    // Устанавливаем курсы по умолчанию
    setCurrencyRate("USD", "RUB", 77.7586);
    setCurrencyRate("RUB", "USD", 0.0129);
    
    qDebug() << "Курсы валют по умолчанию установлены";
}

void FleetDatabase::createSampleData()
{
    qDebug() << "Создание тестовых данных...";
    
    // Добавляем проекты
    const auto project1 = std::make_shared<Project>("ЖК «Солнечный»");
    const auto project2 = std::make_shared<Project>("БЦ «Меридиан»");
    const auto project3 = std::make_shared<Project>("Школа №15");
    
    addProject(project1);
    addProject(project2);
    addProject(project3);
    
    // Добавляем технику
    const auto machine1 = std::make_shared<Machine>("Экскаватор CAT 320D", "Экскаватор", "CAT320D-2019-0847", 2019, Money(1000, Currency::USD));
    machine1->setStatus(MachineStatus::OnSite);
    machine1->setCurrentProject("ЖК «Солнечный»");
    machine1->setAssignedDate(QDate(2026, 1, 20));
    addMachine(machine1);

    const auto machine2 = std::make_shared<Machine>("Бульдозер Komatsu D65", "Бульдозер", "KOM-D65-2020-1123", 2020, Money(12300000, Currency::RUB));
    machine2->setStatus(MachineStatus::Available);
    addMachine(machine2);

    const auto machine3 = std::make_shared<Machine>("Кран башенный КБ-403", "Кран", "KB403-2018-0291", 2018, Money(15700000, Currency::RUB));
    machine3->setStatus(MachineStatus::OnSite);
    machine3->setCurrentProject("БЦ «Меридиан»");
    addMachine(machine3);

    const auto machine4 = std::make_shared<Machine>("Автокран Liebherr LTM 1050", "Автокран", "LTM1050-2021-0055", 2021, Money(22100000, Currency::RUB));
    machine4->setStatus(MachineStatus::InRepair);
    addMachine(machine4);

    const auto machine5 = std::make_shared<Machine>("Погрузчик JCB 531-70", "Погрузчик", "JCB531-2019-0782", 2019, Money(4200000, Currency::RUB));
    machine5->setStatus(MachineStatus::Available);
    addMachine(machine5);

    const auto machine6 = std::make_shared<Machine>("Экскаватор-погрузчик JCB 3CX", "Экскаватор-погрузчик", "JCB3CX-2020-0394", 2020, Money(5800000, Currency::RUB));
    machine6->setStatus(MachineStatus::OnSite);
    machine6->setCurrentProject("ЖК «Солнечный»");
    addMachine(machine6);

    const auto machine7 = std::make_shared<Machine>("Самосвал КАМАЗ-6520", "Самосвал", "KMZ6520-2017-1847", 2017, Money(3900000, Currency::RUB));
    machine7->setStatus(MachineStatus::Available);
    addMachine(machine7);

    const auto machine8 = std::make_shared<Machine>("Бетономешалка MAN TGS", "Бетономешалка", "MAN-TGS-2019-0621", 2019, Money(7200000, Currency::RUB));
    machine8->setStatus(MachineStatus::OnSite);
    machine8->setCurrentProject("Школа №15");
    addMachine(machine8);

    const auto machine9 = std::make_shared<Machine>("Каток BOMAG BW 213", "Каток", "BOMAG213-2018-0183", 2018, Money(6100000, Currency::RUB));
    machine9->setStatus(MachineStatus::InRepair);
    addMachine(machine9);

    const auto machine10 = std::make_shared<Machine>("Грейдер ДЗ-98", "Грейдер", "DZ98-2016-0095", 2016, Money(2800000, Currency::RUB));
    machine10->setStatus(MachineStatus::Decommissioned);
    addMachine(machine10);

    const auto machine11 = std::make_shared<Machine>("Виброплита Wacker Neuson", "Виброплита", "WN-VP-2022-0012", 2022, Money(320000, Currency::RUB));
    machine11->setStatus(MachineStatus::Available);
    addMachine(machine11);

    const auto machine12 = std::make_shared<Machine>("Компрессор Atlas Copco", "Компрессор", "AC-XAS-2021-0487", 2021, Money(890000, Currency::RUB));
    machine12->setStatus(MachineStatus::OnSite);
    machine12->setCurrentProject("БЦ «Меридиан»");
    addMachine(machine12);
    
    qDebug() << "Тестовые данные созданы";
}

// ===== ОПЕРАЦИИ С ТЕХНИКОЙ =====

bool FleetDatabase::addMachine(const MachinePtr& machine)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO machines (name, type, serial_number, year_of_manufacture, status, cost, currency, current_project, assigned_date)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(machine->getName());
    query.addBindValue(machine->getType());
    query.addBindValue(machine->getSerialNumber());
    query.addBindValue(machine->getYearOfManufacture());
    query.addBindValue(Machine::statusToString(machine->getStatus()));
    query.addBindValue(machine->getCost().getAmount());
    query.addBindValue(Money::getCurrencyName(machine->getCost().getCurrency()));
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
            status = ?, cost = ?, currency = ?, current_project = ?, assigned_date = ?
        WHERE id = ?
    )");
    
    query.addBindValue(machine->getName());
    query.addBindValue(machine->getType());
    query.addBindValue(machine->getSerialNumber());
    query.addBindValue(machine->getYearOfManufacture());
    query.addBindValue(Machine::statusToString(machine->getStatus()));
    query.addBindValue(machine->getCost().getAmount());
    query.addBindValue(Money::getCurrencyName(machine->getCost().getCurrency()));
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
        
        // Загружаем стоимость с валютой
        double amount = query.value("cost").toDouble();
        QString currencyStr = query.value("currency").toString();
        Currency currency = Money::currencyFromString(currencyStr);
        machine->setCost(Money(amount, currency));
        
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
    
    // Загружаем стоимость с валютой
    double amount = query.value("cost").toDouble();
    QString currencyStr = query.value("currency").toString();
    Currency currency = Money::currencyFromString(currencyStr);
    machine->setCost(Money(amount, currency));
    
    machine->setCurrentProject(query.value("current_project").toString());

    const QString dateStr = query.value("assigned_date").toString();
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
        
        // Загружаем стоимость с валютой
        double amount = query.value("cost").toDouble();
        QString currencyStr = query.value("currency").toString();
        Currency currency = Money::currencyFromString(currencyStr);
        machine->setCost(Money(amount, currency));
        
        machine->setCurrentProject(query.value("current_project").toString());
        
        QString dateStr = query.value("assigned_date").toString();
        if (!dateStr.isEmpty()) {
            machine->setAssignedDate(QDate::fromString(dateStr, Qt::ISODate));
        }
        
        machines.append(machine);
    }
    
    return machines;
}

QVector<MachinePtr> FleetDatabase::getMachinesByProject(const QString& projectName)
{
    QVector<MachinePtr> machines;
    QSqlQuery query;
    query.prepare("SELECT * FROM machines WHERE current_project = ? ORDER BY id");
    query.addBindValue(projectName);
    
    if (!query.exec()) {
        qWarning() << "Ошибка получения техники по проекту:" << query.lastError().text();
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
        
        // Загружаем стоимость с валютой
        double amount = query.value("cost").toDouble();
        QString currencyStr = query.value("currency").toString();
        Currency currency = Money::currencyFromString(currencyStr);
        machine->setCost(Money(amount, currency));
        
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
        INSERT INTO projects (name, description)
        VALUES (?, ?)
    )");
    
    query.addBindValue(project->getName());
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
        SET name = ?, description = ?
        WHERE id = ?
    )");
    
    query.addBindValue(project->getName());
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
        const int count = query.value("count").toInt();
        
        if (status == "Свободна") stats.available = count;
        else if (status == "На объекте") stats.onSite = count;
        else if (status == "В ремонте") stats.inRepair = count;
        else if (status == "Списана") stats.decommissioned = count;
    }
    
    stats.total = stats.available + stats.onSite + stats.inRepair + stats.decommissioned;
    return stats;
}

// ===== УПРАВЛЕНИЕ КУРСАМИ ВАЛЮТ =====

bool FleetDatabase::setCurrencyRate(const QString& fromCurrency, const QString& toCurrency, double rate)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT OR REPLACE INTO currency_rates (from_currency, to_currency, rate)
        VALUES (?, ?, ?)
    )");
    
    query.addBindValue(fromCurrency);
    query.addBindValue(toCurrency);
    query.addBindValue(rate);
    
    if (!query.exec()) {
        qWarning() << "Ошибка сохранения курса валют:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Курс валют сохранен:" << fromCurrency << "→" << toCurrency << "=" << rate;
    return true;
}

double FleetDatabase::getCurrencyRate(const QString& fromCurrency, const QString& toCurrency)
{
    QSqlQuery query;
    query.prepare("SELECT rate FROM currency_rates WHERE from_currency = ? AND to_currency = ?");
    query.addBindValue(fromCurrency);
    query.addBindValue(toCurrency);
    
    if (query.exec() && query.next()) {
        return query.value("rate").toDouble();
    }
    
    return 1.0; // По умолчанию
}

void FleetDatabase::loadCurrencyRates()
{
    // Методы Money::getExchangeRate() теперь автоматически получают курсы из БД
    qDebug() << "Курсы валют готовы к использованию из БД";
}

QMap<QString, double> FleetDatabase::getAllCurrencyRates()
{
    QMap<QString, double> rates;
    QSqlQuery query("SELECT from_currency, to_currency, rate FROM currency_rates");
    
    while (query.next()) {
        QString from = query.value("from_currency").toString();
        QString to = query.value("to_currency").toString();
        double rate = query.value("rate").toDouble();
        
        QString key = from + "_" + to;
        rates[key] = rate;
    }
    
    return rates;
}
