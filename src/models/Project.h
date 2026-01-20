#pragma once

#include <QString>
#include <memory>

/**
 * @brief Класс, представляющий строительный проект/объект
 */
class Project {
public:
    Project();
    Project(const QString& name, const QString& description = "");
    
    // Геттеры
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }
    
    // Сеттеры
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setDescription(const QString& description) { m_description = description; }

private:
    int m_id;                   // ID в базе данных
    QString m_name;              // Название проекта
    QString m_description;       // Описание проекта
};

using ProjectPtr = std::shared_ptr<Project>;
