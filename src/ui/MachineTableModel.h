#pragma once

#include <QAbstractTableModel>
#include "../models/Machine.h"
#include <QVector>

/**
 * @brief Модель таблицы для отображения списка техники
 * 
 * Реализует QAbstractTableModel для управления данными в QTableView.
 * Поддерживает фильтрацию по статусу техники.
 */
class MachineTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Конструктор модели таблицы
     * @param parent Родительский объект
     */
    explicit MachineTableModel(QObject *parent = nullptr);
    
    // Переопределённые методы QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    
    /**
     * @brief Загрузить данные из базы
     */
    void loadData();
    
    /**
     * @brief Получить машину по индексу строки
     * @param row Номер строки
     * @return Указатель на объект Machine или nullptr
     */
    MachinePtr getMachine(int row) const;
    
    /**
     * @brief Установить фильтр по статусу
     * @param status Статус для фильтрации (если -1, то показать все)
     */
    void setStatusFilter(int statusIndex);
    
    /**
     * @brief Получить индекс строки по ID техники
     * @param machineId ID техники
     * @return Индекс строки или -1 если не найдено
     */
    int getRowById(int machineId) const;
    
    /**
     * @brief Установить видимость колонки
     * @param column Номер колонки
     * @param visible true - показать, false - скрыть
     */
    void setColumnVisible(int column, bool visible);
    
    /**
     * @brief Проверить видимость колонки
     * @param column Номер колонки
     * @return true если колонка видима
     */
    bool isColumnVisible(int column) const;
    
    /**
     * @brief Получить список всех колонок
     * @return QList с парами {index, name, isVisible}
     */
    QList<std::tuple<int, QString, bool>> getColumnsInfo() const;

private:
    /**
     * @brief Применить фильтр к данным
     */
    void applyFilter();
    
    QVector<MachinePtr> m_allMachines;      // Все машины
    QVector<MachinePtr> m_machines;          // Отфильтрованные машины (отображаемые)
    int m_currentStatusFilter;               // Текущий фильтр (-1 = все)
    
    // Заголовки столбцов
    QStringList m_headers;
    
    // Параметры сортировки
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
    
    // Видимость столбцов (индекс -> видимость)
    QVector<bool> m_columnVisibility;
    
    /**
     * @brief Получить реальный индекс колонки с учётом скрытых колонок
     * @param displayColumn Видимый индекс колонки
     * @return Реальный индекс колонки (-1 если не найдено)
     */
    int getActualColumnIndex(int displayColumn) const;
};
