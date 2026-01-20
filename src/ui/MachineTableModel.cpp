#include "MachineTableModel.h"
#include "../database/FleetDatabase.h"
#include <QBrush>
#include <QColor>
#include <algorithm>

MachineTableModel::MachineTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_currentStatusFilter(-1) // -1 означает показать все
{
    m_headers << "Название" << "Статус" << "Текущий проект";
    
    // Инициализируем расширенный список заголовков с полной информацией
    // Все столбцы видимы по умолчанию
    m_columnVisibility.resize(m_headers.size());
    for (int i = 0; i < m_headers.size(); ++i) {
        m_columnVisibility[i] = true;
    }
}

int MachineTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_machines.size();
}

int MachineTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    
    int visibleCount = 0;
    for (bool visible : m_columnVisibility) {
        if (visible) visibleCount++;
    }
    return visibleCount;
}

int MachineTableModel::getActualColumnIndex(int displayColumn) const
{
    int visibleCount = 0;
    for (int i = 0; i < m_columnVisibility.size(); ++i) {
        if (m_columnVisibility[i]) {
            if (visibleCount == displayColumn) {
                return i;
            }
            visibleCount++;
        }
    }
    return -1;
}

QVariant MachineTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_machines.size()) {
        return QVariant();
    }
    
    // Получить реальный индекс столбца
    int actualColumn = getActualColumnIndex(index.column());
    if (actualColumn == -1) {
        return QVariant();
    }
    
    auto machine = m_machines[index.row()];
    
    // Отображение данных
    if (role == Qt::DisplayRole) {
        switch (actualColumn) {
            case 0: return machine->getName();
            case 1: return Machine::statusToString(machine->getStatus());
            case 2: return machine->getCurrentProject().isEmpty() ? "—" : machine->getCurrentProject();
            default: return QVariant();
        }
    }
    
    // Цветовое кодирование статуса
    if (role == Qt::BackgroundRole && actualColumn == 1) {
        switch (machine->getStatus()) {
            case MachineStatus::Available:
                return QBrush(QColor(76, 175, 80, 50)); // Зелёный (свободна)
            case MachineStatus::OnSite:
                return QBrush(QColor(33, 150, 243, 50)); // Синий (на объекте)
            case MachineStatus::InRepair:
                return QBrush(QColor(255, 152, 0, 50)); // Оранжевый (в ремонте)
            case MachineStatus::Decommissioned:
                return QBrush(QColor(244, 67, 54, 50)); // Красный (списана)
            default:
                return QVariant();
        }
    }
    
    // Цвет текста для статуса
    if (role == Qt::ForegroundRole && actualColumn == 1) {
        switch (machine->getStatus()) {
            case MachineStatus::Available:
                return QBrush(QColor(76, 175, 80)); // Зелёный
            case MachineStatus::OnSite:
                return QBrush(QColor(33, 150, 243)); // Синий
            case MachineStatus::InRepair:
                return QBrush(QColor(255, 152, 0)); // Оранжевый
            case MachineStatus::Decommissioned:
                return QBrush(QColor(244, 67, 54)); // Красный
            default:
                return QVariant();
        }
    }
    
    return QVariant();
}

QVariant MachineTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        int actualColumn = getActualColumnIndex(section);
        if (actualColumn >= 0 && actualColumn < m_headers.size()) {
            return m_headers[actualColumn];
        }
    }
    return QVariant();
}

void MachineTableModel::loadData()
{
    beginResetModel();
    m_allMachines = FleetDatabase::instance().getAllMachines();
    applyFilter();
    endResetModel();
}

MachinePtr MachineTableModel::getMachine(int row) const
{
    if (row >= 0 && row < m_machines.size()) {
        return m_machines[row];
    }
    return nullptr;
}

void MachineTableModel::setStatusFilter(int statusIndex)
{
    beginResetModel();
    m_currentStatusFilter = statusIndex;
    applyFilter();
    endResetModel();
}

void MachineTableModel::applyFilter()
{
    m_machines.clear();
    
    if (m_currentStatusFilter == -1 || m_currentStatusFilter == 0) {
        // Показать все
        m_machines = m_allMachines;
    } else {
        // Фильтрация по статусу
        MachineStatus targetStatus;
        switch (m_currentStatusFilter) {
            case 1: targetStatus = MachineStatus::Available; break;
            case 2: targetStatus = MachineStatus::OnSite; break;
            case 3: targetStatus = MachineStatus::InRepair; break;
            case 4: targetStatus = MachineStatus::Decommissioned; break;
            default:
                m_machines = m_allMachines;
                return;
        }
        
        for (const auto& machine : m_allMachines) {
            if (machine->getStatus() == targetStatus) {
                m_machines.append(machine);
            }
        }
    }
    
    // Применить сортировку после фильтрации
    if (m_sortColumn >= 0) {
        sort(m_sortColumn, m_sortOrder);
    }
}

void MachineTableModel::sort(int column, Qt::SortOrder order)
{
    // Получить реальный индекс столбца
    int actualColumn = getActualColumnIndex(column);
    if (actualColumn < 0 || actualColumn >= m_headers.size()) {
        return;
    }
    
    // Сохранить параметры сортировки
    m_sortColumn = actualColumn;
    m_sortOrder = order;
    
    // Выполнить сортировку
    emit layoutAboutToBeChanged();
    
    std::sort(m_machines.begin(), m_machines.end(),
        [column, order](const MachinePtr& a, const MachinePtr& b) {
            bool result = false;
            switch (column) {
                case 0: // Название
                    result = a->getName().toLower() < b->getName().toLower();
                    break;
                case 1: // Статус
                    result = static_cast<int>(a->getStatus()) < static_cast<int>(b->getStatus());
                    break;
                case 2: // Текущий проект
                    result = a->getCurrentProject().toLower() < b->getCurrentProject().toLower();
                    break;
                default:
                    return false;
            }
            return order == Qt::AscendingOrder ? result : !result;
        });
    
    emit layoutChanged();
}

int MachineTableModel::getRowById(int machineId) const
{
    for (int i = 0; i < m_machines.size(); ++i) {
        if (m_machines[i]->getId() == machineId) {
            return i;
        }
    }
    return -1;
}

void MachineTableModel::setColumnVisible(int column, bool visible)
{
    if (column < 0 || column >= m_columnVisibility.size()) {
        return;
    }
    
    if (m_columnVisibility[column] == visible) {
        return; // Нет изменений
    }
    
    beginResetModel();
    m_columnVisibility[column] = visible;
    endResetModel();
}

bool MachineTableModel::isColumnVisible(int column) const
{
    if (column < 0 || column >= m_columnVisibility.size()) {
        return false;
    }
    return m_columnVisibility[column];
}

QList<std::tuple<int, QString, bool>> MachineTableModel::getColumnsInfo() const
{
    QList<std::tuple<int, QString, bool>> result;
    for (int i = 0; i < m_headers.size(); ++i) {
        result.append(std::make_tuple(i, m_headers[i], m_columnVisibility[i]));
    }
    return result;
}
