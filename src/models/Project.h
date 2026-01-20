#pragma once

#include <QString>
#include <QDate>
#include <QVector>
#include <memory>

/**
 * @brief Класс, представляющий строительный проект/объект
 * 
 * Проект может иметь назначенную технику, даты начала и окончания работ.
 */
class Project {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Project();
    
    /**
     * @brief Конструктор с параметрами
     * @param name Название проекта
     * @param startDate Дата начала
     * @param endDate Дата окончания (может быть пустой)
     */
    Project(const QString& name, 
            const QDate& startDate,
            const QDate& endDate = QDate());
    
    // Геттеры
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QDate getStartDate() const { return m_startDate; }
    QDate getEndDate() const { return m_endDate; }
    QString getDescription() const { return m_description; }
    bool isActive() const;
    
    // Сеттеры
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setStartDate(const QDate& date) { m_startDate = date; }
    void setEndDate(const QDate& date) { m_endDate = date; }
    void setDescription(const QString& description) { m_description = description; }

private:
    int m_id;                   // ID в базе данных
    QString m_name;              // Название проекта
    QDate m_startDate;          // Дата начала
    QDate m_endDate;            // Дата окончания (если пустая - проект активен)
    QString m_description;       // Описание проекта
};

using ProjectPtr = std::shared_ptr<Project>;
