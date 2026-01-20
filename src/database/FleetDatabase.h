#pragma once

#include "../models/Machine.h"
#include "../models/Project.h"
#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <memory>

/**
 * @brief Класс для работы с базой данных парка техники
 * 
 * Отвечает за создание таблиц, добавление, удаление, обновление
 * и получение данных о технике и проектах.
 */
class FleetDatabase {
public:
    /**
     * @brief Получить singleton-экземпляр базы данных
     * @return Ссылка на экземпляр FleetDatabase
     */
    static FleetDatabase& instance();
    
    /**
     * @brief Инициализация базы данных
     * @param dbPath Путь к файлу базы данных
     * @return true если инициализация успешна, иначе false
     */
    bool initialize(const QString& dbPath = "fleet.db");
    
    /**
     * @brief Закрыть соединение с базой данных
     */
    void close();
    
    // ===== ОПЕРАЦИИ С ТЕХНИКОЙ =====
    
    /**
     * @brief Добавить новую технику в базу
     * @param machine Указатель на объект Machine
     * @return true если добавление успешно, иначе false
     */
    bool addMachine(MachinePtr machine);
    
    /**
     * @brief Обновить существующую запись о технике
     * @param machine Указатель на объект Machine
     * @return true если обновление успешно, иначе false
     */
    bool updateMachine(MachinePtr machine);
    
    /**
     * @brief Удалить технику из базы
     * @param machineId ID техники
     * @return true если удаление успешно, иначе false
     */
    bool deleteMachine(int machineId);
    
    /**
     * @brief Получить всю технику из базы
     * @return Вектор указателей на объекты Machine
     */
    QVector<MachinePtr> getAllMachines();
    
    /**
     * @brief Получить технику по ID
     * @param machineId ID техники
     * @return Указатель на объект Machine или nullptr
     */
    MachinePtr getMachineById(int machineId);
    
    /**
     * @brief Получить технику по статусу
     * @param status Статус техники
     * @return Вектор указателей на объекты Machine
     */
    QVector<MachinePtr> getMachinesByStatus(MachineStatus status);
    
    // ===== ОПЕРАЦИИ С ПРОЕКТАМИ =====
    
    /**
     * @brief Добавить новый проект в базу
     * @param project Указатель на объект Project
     * @return true если добавление успешно, иначе false
     */
    bool addProject(ProjectPtr project);
    
    /**
     * @brief Обновить существующий проект
     * @param project Указатель на объект Project
     * @return true если обновление успешно, иначе false
     */
    bool updateProject(ProjectPtr project);
    
    /**
     * @brief Удалить проект из базы
     * @param projectId ID проекта
     * @return true если удаление успешно, иначе false
     */
    bool deleteProject(int projectId);
    
    /**
     * @brief Получить все проекты
     * @return Вектор указателей на объекты Project
     */
    QVector<ProjectPtr> getAllProjects();
    
    /**
     * @brief Получить проект по ID
     * @param projectId ID проекта
     * @return Указатель на объект Project или nullptr
     */
    ProjectPtr getProjectById(int projectId);
    
    // ===== СТАТИСТИКА =====
    
    /**
     * @brief Получить количество техники по статусам
     * @return Структура с количеством техники в каждом статусе
     */
    struct Statistics {
        int total;          // Всего техники
        int available;      // Свободна
        int onSite;         // На объектах
        int inRepair;       // В ремонте
        int decommissioned; // Списана
    };
    
    Statistics getStatistics();

private:
    FleetDatabase(); // Приватный конструктор для singleton
    ~FleetDatabase();
    
    FleetDatabase(const FleetDatabase&) = delete;
    FleetDatabase& operator=(const FleetDatabase&) = delete;
    
    /**
     * @brief Создать таблицы в базе данных
     * @return true если создание успешно, иначе false
     */
    bool createTables();
    
    /**
     * @brief Создать тестовые данные (для демонстрации)
     */
    void createSampleData();
    
    QSqlDatabase m_database;
    bool m_initialized;
};
