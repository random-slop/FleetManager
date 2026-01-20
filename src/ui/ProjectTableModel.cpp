#include "ProjectTableModel.h"
#include "../database/FleetDatabase.h"

ProjectTableModel::ProjectTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    refresh();
}

int ProjectTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_projects.size();
}

int ProjectTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant ProjectTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_projects.size())
        return QVariant();

    const auto& project = m_projects[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case Id: return project->getId();
            case Name: return project->getName();
            case Description: return project->getDescription();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == Id)
            return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant ProjectTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
        case Id: return "ID";
        case Name: return "Название";
        case Description: return "Описание";
    }

    return QVariant();
}

void ProjectTableModel::refresh()
{
    beginResetModel();
    m_projects = FleetDatabase::instance().getAllProjects();
    endResetModel();
}

ProjectPtr ProjectTableModel::getProject(int row) const
{
    if (row < 0 || row >= m_projects.size())
        return nullptr;
    return m_projects[row];
}
