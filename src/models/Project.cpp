#include "Project.h"

Project::Project() : m_id(-1) {}

Project::Project(const QString& name, const QString& description)
    : m_id(-1), m_name(name), m_description(description) {}
