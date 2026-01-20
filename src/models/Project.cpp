#include "Project.h"

Project::Project()
    : m_id(-1)
    , m_startDate(QDate::currentDate())
{
}

Project::Project(const QString& name, 
                 const QDate& startDate,
                 const QDate& endDate)
    : m_id(-1)
    , m_name(name)
    , m_startDate(startDate)
    , m_endDate(endDate)
{
}

bool Project::isActive() const
{
    // Проект активен, если дата окончания не установлена или ещё не наступила
    return !m_endDate.isValid() || m_endDate >= QDate::currentDate();
}
