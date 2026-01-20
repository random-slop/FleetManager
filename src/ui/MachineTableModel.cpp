#include "MachineTableModel.h"
#include "../database/FleetDatabase.h"
#include <QBrush>
#include <QColor>

MachineTableModel::MachineTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_currentStatusFilter(-1) // -1 означает показать все
{
    m_headers << "Название" << "Тип" << "Серийный номер" 
              << "Год выпуска" << "Статус" << "Стоимость" << "Текущий проект";
}

int MachineTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_machines.size();
}

int MachineTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_headers.size();
}

QVariant MachineTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_machines.size()) {
        return QVariant();
    }
    
    auto machine = m_machines[index.row()];
    
    // Отображение данных
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return machine->getName();
            case 1: return machine->getType();
            case 2: return machine->getSerialNumber();
            case 3: return machine->getYearOfManufacture();
            case 4: return Machine::statusToString(machine->getStatus());
            case 5: return QString("%1 ₽").arg(machine->getCost(), 0, 'f', 0);
            case 6: return machine->getCurrentProject().isEmpty() ? "—" : machine->getCurrentProject();
            default: return QVariant();
        }
    }
    
    // Цветовое кодирование статуса
    if (role == Qt::BackgroundRole && index.column() == 4) {
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
    if (role == Qt::ForegroundRole && index.column() == 4) {
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
        if (section >= 0 && section < m_headers.size()) {
            return m_headers[section];
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
