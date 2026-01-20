#include "MachineTableModel.h"
#include "../database/FleetDatabase.h"
#include "../models/Money.h"
#include <QBrush>
#include <QColor>
#include <algorithm>

MachineTableModel::MachineTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_currentStatusFilter(-1) // -1 означает показать все
{
    m_headers << "Название" << "Статус" << "Текущий проект" << "Тип техники" << "Серийный номер" << "Год выпуска" << "Стоимость" << "Назначен с";

    m_columnVisibility.resize(m_headers.size());
    m_columnVisibility[0] = true;  // Название
    m_columnVisibility[1] = true;  // Статус
    m_columnVisibility[2] = true;  // Текущий проект
    m_columnVisibility[3] = false; // Тип техники
    m_columnVisibility[4] = false; // Серийный номер
    m_columnVisibility[5] = false; // Год выпуска
    m_columnVisibility[6] = false; // Стоимость
    m_columnVisibility[7] = false; // Назначен с
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
    for (const bool visible : m_columnVisibility)
        if (visible) visibleCount++;
    return visibleCount;
}

int MachineTableModel::getActualColumnIndex(const int displayColumn) const
{
    int visibleCount = 0;
    for (int i = 0; i < m_columnVisibility.size(); ++i)
        if (m_columnVisibility[i]) {
            if (visibleCount == displayColumn)
                return i;
            visibleCount++;
        }
    return -1;
}

QVariant MachineTableModel::data(const QModelIndex &index, const int role) const
{
    if (!index.isValid() || index.row() >= m_machines.size())
        return QVariant();

    const int actualColumn = getActualColumnIndex(index.column());
    if (actualColumn == -1)
        return QVariant();

    const auto machine = m_machines[index.row()];
    
    if (role == Qt::DisplayRole) {
        switch (actualColumn) {
            case 0: return machine->getName();
            case 1: return Machine::statusToString(machine->getStatus());
            case 2: return machine->getCurrentProject().isEmpty() ? "—" : machine->getCurrentProject();
            case 3: return machine->getType();
            case 4: return machine->getSerialNumber();
            case 5: return QString::number(machine->getYearOfManufacture());
            case 6: {
                const Money cost = machine->getCost();
                QString costText = cost.toString();
                if (cost.getCurrency() != Currency::RUB) {
                    const Money rubles = cost.convertTo(Currency::RUB);
                    costText += QString(" (%1)").arg(rubles.toString());
                }
                return costText;
            }
            case 7: return machine->getAssignedDate().isValid() ? machine->getAssignedDate().toString("dd.MM.yyyy") : "—";
            default: return QVariant();
        }
    }
    
    if (role == Qt::BackgroundRole && actualColumn == 1)
        switch (machine->getStatus()) {
        case MachineStatus::Available:
            return QBrush(QColor(76, 175, 80, 50)); // Зелёный (свободна)
        case MachineStatus::OnSite:
            return QBrush(QColor(33, 150, 243, 50)); // Синий (на объекте)
        case MachineStatus::InRepair:
            return QBrush(QColor(255, 152, 0, 50)); // Оранжевый (в ремонте)
        case MachineStatus::Decommissioned:
            return QBrush(QColor(244, 67, 54, 50)); // Красный (списана)
        default: return QVariant();
        }

    if (role == Qt::ForegroundRole && actualColumn == 1)
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

    return QVariant();
}

QVariant MachineTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        const int actualColumn = getActualColumnIndex(section);
        if (actualColumn >= 0 && actualColumn < m_headers.size())
            return m_headers[actualColumn];
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

MachinePtr MachineTableModel::getMachine(const int row) const
{
    if (row >= 0 && row < m_machines.size()) return m_machines[row];
    return nullptr;
}

void MachineTableModel::setStatusFilter(const int statusIndex)
{
    beginResetModel();
    m_currentStatusFilter = statusIndex;
    applyFilter();
    endResetModel();
}

void MachineTableModel::applyFilter()
{
    m_machines.clear();
    
    if (m_currentStatusFilter == -1 || m_currentStatusFilter == 0)
        m_machines = m_allMachines;
    else {
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
        
        for (const auto& machine : m_allMachines)
            if (machine->getStatus() == targetStatus)
                m_machines.append(machine);
    }
    
    if (m_sortColumn >= 0) sort(m_sortColumn, m_sortOrder);
}

void MachineTableModel::sort(const int column, Qt::SortOrder order)
{
    // Получить реальный индекс столбца
    int actualColumn = getActualColumnIndex(column);
    if (actualColumn < 0 || actualColumn >= m_headers.size())
        return;

    // Сохранить параметры сортировки
    m_sortColumn = actualColumn;
    m_sortOrder = order;
    
    // Выполнить сортировку
    emit layoutAboutToBeChanged();
    
    std::ranges::sort(m_machines,
                      [actualColumn, order](const MachinePtr& a, const MachinePtr& b) {
                          bool result = false;
                          switch (actualColumn) {
                          case 0: // Название
                              result = a->getName().toLower() < b->getName().toLower();
                              break;
                          case 1: // Статус
                              result = static_cast<int>(a->getStatus()) < static_cast<int>(b->getStatus());
                              break;
                          case 2: // Текущий проект
                              result = a->getCurrentProject().toLower() < b->getCurrentProject().toLower();
                              break;
                          case 3: // Тип техники
                              result = a->getType().toLower() < b->getType().toLower();
                              break;
                          case 4: // Серийный номер
                              result = a->getSerialNumber().toLower() < b->getSerialNumber().toLower();
                              break;
                          case 5: // Год выпуска
                              result = a->getYearOfManufacture() < b->getYearOfManufacture();
                              break;
                          case 6: // Стоимость
                              result = a->getCost().toRubles() < b->getCost().toRubles();
                              break;
                          case 7: // Назначен с
                              result = a->getAssignedDate() < b->getAssignedDate();
                              break;
                          default:
                              return false;
                          }
                          return order == Qt::AscendingOrder ? result : !result;
                      });
    
    emit layoutChanged();
}

int MachineTableModel::getRowById(const int machineId) const
{
    for (int i = 0; i < m_machines.size(); ++i)
        if (m_machines[i]->getId() == machineId)
            return i;
    return -1;
}

void MachineTableModel::setColumnVisible(const int column, const bool visible)
{
    if (column < 0 || column >= m_columnVisibility.size()) return;

    if (m_columnVisibility[column] == visible) return;

    beginResetModel();
    m_columnVisibility[column] = visible;
    endResetModel();
}

bool MachineTableModel::isColumnVisible(const int column) const
{
    if (column < 0 || column >= m_columnVisibility.size()) return false;
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
