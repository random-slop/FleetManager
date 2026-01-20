#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include "../models/Project.h"

/**
 * @brief Модель таблицы для отображения списка проектов
 */
class ProjectTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        Id = 0,
        Name,
        Description,
        ColumnCount
    };

    explicit ProjectTableModel(QObject* parent = nullptr);

    // Методы QAbstractTableModel
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * @brief Обновить данные из базы
     */
    void refresh();

    /**
     * @brief Получить проект по индексу строки
     */
    ProjectPtr getProject(int row) const;

private:
    QVector<ProjectPtr> m_projects;
};
