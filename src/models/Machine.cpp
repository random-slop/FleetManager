#include "Machine.h"

Machine::Machine()
    : m_id(-1)
    , m_yearOfManufacture(2020)
    , m_status(MachineStatus::Available)
    , m_cost(0.0)
{
}

Machine::Machine(const QString& name, 
                 const QString& type,
                 const QString& serialNumber,
                 int yearOfManufacture,
                 double cost)
    : m_id(-1)
    , m_name(name)
    , m_type(type)
    , m_serialNumber(serialNumber)
    , m_yearOfManufacture(yearOfManufacture)
    , m_status(MachineStatus::Available)
    , m_cost(cost)
{
}

QString Machine::statusToString(MachineStatus status)
{
    switch (status) {
        case MachineStatus::Available:
            return "Свободна";
        case MachineStatus::OnSite:
            return "На объекте";
        case MachineStatus::InRepair:
            return "В ремонте";
        case MachineStatus::Decommissioned:
            return "Списана";
        default:
            return "Неизвестно";
    }
}

MachineStatus Machine::stringToStatus(const QString& str)
{
    if (str == "Свободна") return MachineStatus::Available;
    if (str == "На объекте") return MachineStatus::OnSite;
    if (str == "В ремонте") return MachineStatus::InRepair;
    if (str == "Списана") return MachineStatus::Decommissioned;
    
    return MachineStatus::Available; // По умолчанию
}
