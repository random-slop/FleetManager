#pragma once

#include <QString>
#include <QDate>
#include <memory>
#include "Money.h"

/**
 * @brief Статус техники в парке
 */
enum class MachineStatus {
    Available,      // Свободна
    OnSite,         // На объекте
    InRepair,       // В ремонте
    Decommissioned  // Списана
};

/**
 * @brief Класс, представляющий единицу техники в парке
 * 
 * Содержит всю необходимую информацию о машине: название, тип,
 * серийный номер, год выпуска, текущий статус и стоимость.
 */
class Machine {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Machine();
    
    /**
     * @brief Конструктор с параметрами
     * @param name Название техники
     * @param type Тип техники (Экскаватор, Кран и т.д.)
     * @param serialNumber Серийный номер
     * @param yearOfManufacture Год выпуска
     * @param cost Стоимость
     */
    Machine(const QString& name, 
            const QString& type,
            const QString& serialNumber,
            int yearOfManufacture,
            const Money& cost);
    
    // Геттеры
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getType() const { return m_type; }
    QString getSerialNumber() const { return m_serialNumber; }
    int getYearOfManufacture() const { return m_yearOfManufacture; }
    MachineStatus getStatus() const { return m_status; }
    Money getCost() const { return m_cost; }
    QString getCurrentProject() const { return m_currentProject; }
    QDate getAssignedDate() const { return m_assignedDate; }
    int getMileage() const { return m_mileage; }
    QDate getNextMaintenanceDate() const { return m_nextMaintenanceDate; }
    QDate getPurchaseDate() const { return m_purchaseDate; }
    int getWarrantyPeriod() const { return m_warrantyPeriod; }
    
    // Сеттеры
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setType(const QString& type) { m_type = type; }
    void setSerialNumber(const QString& serialNumber) { m_serialNumber = serialNumber; }
    void setYearOfManufacture(int year) { m_yearOfManufacture = year; }
    void setStatus(MachineStatus status) { m_status = status; }
    void setCost(const Money& cost) { m_cost = cost; }
    void setCurrentProject(const QString& project) { m_currentProject = project; }
    void setAssignedDate(const QDate& date) { m_assignedDate = date; }
    void setMileage(int mileage) { m_mileage = mileage; }
    void setNextMaintenanceDate(const QDate& date) { m_nextMaintenanceDate = date; }
    void setPurchaseDate(const QDate& date) { m_purchaseDate = date; }
    void setWarrantyPeriod(const int months) { m_warrantyPeriod = months; }
    
    /**
     * @brief Преобразует статус в строковое представление
     * @param status Статус техники
     * @return Строковое представление статуса
     */
    static QString statusToString(MachineStatus status);
    
    /**
     * @brief Преобразует строку в статус
     * @param str Строка со статусом
     * @return Статус техники
     */
    static MachineStatus stringToStatus(const QString& str);

private:
    int m_id;                           // ID в базе данных
    QString m_name;                      // Название (например, "Экскаватор CAT 320D")
    QString m_type;                      // Тип (Экскаватор, Кран, Бульдозер и т.д.)
    QString m_serialNumber;              // Серийный номер (например, "CAT320D-2019-0847")
    int m_yearOfManufacture;            // Год выпуска
    MachineStatus m_status;             // Текущий статус
    Money m_cost;                       // Стоимость
    QString m_currentProject;            // Текущий проект (если назначена)
    QDate m_assignedDate;               // Дата назначения на проект
    int m_mileage;                      // Пробег
    QDate m_nextMaintenanceDate;        // Дата следующего обслуживания
    QDate m_purchaseDate;               // Дата покупки
    int m_warrantyPeriod;               // Гарантийный срок
};

using MachinePtr = std::shared_ptr<Machine>;
