#pragma once

#include <QString>
#include <QDate>
#include <memory>

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
            double cost);
    
    // Геттеры
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getType() const { return m_type; }
    QString getSerialNumber() const { return m_serialNumber; }
    int getYearOfManufacture() const { return m_yearOfManufacture; }
    MachineStatus getStatus() const { return m_status; }
    double getCost() const { return m_cost; }
    QString getCurrentProject() const { return m_currentProject; }
    QDate getAssignedDate() const { return m_assignedDate; }
    
    // Сеттеры
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setType(const QString& type) { m_type = type; }
    void setSerialNumber(const QString& serialNumber) { m_serialNumber = serialNumber; }
    void setYearOfManufacture(int year) { m_yearOfManufacture = year; }
    void setStatus(MachineStatus status) { m_status = status; }
    void setCost(double cost) { m_cost = cost; }
    void setCurrentProject(const QString& project) { m_currentProject = project; }
    void setAssignedDate(const QDate& date) { m_assignedDate = date; }
    
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
    double m_cost;                      // Стоимость в рублях
    QString m_currentProject;            // Текущий проект (если назначена)
    QDate m_assignedDate;               // Дата назначения на проект
};

using MachinePtr = std::shared_ptr<Machine>;
