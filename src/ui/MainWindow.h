#pragma once

#include <QMainWindow>
#include <QPushButton>
#include "../models/Machine.h"
#include "../models/Project.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MachineTableModel;
class ProjectTableModel;
class AssignMachineDialog;
class QTableView;
class QLabel;
class QVBoxLayout;
class QComboBox;
class QStackedWidget;

/**
 * @brief Главное окно приложения "Парк техники"
 * 
 * Содержит меню, toolbar, таблицу с техникой и панель с деталями.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~MainWindow();

private slots:
    // Слоты для переключения режимов
    void showFleetView();
    void showProjectsView();

    // Слоты для действий меню и toolbar
    void onAddMachine();
    void onEditMachine();
    void onDeleteMachine();
    
    void onAddProject();
    void onEditProject();
    void onDeleteProject();

    void onAssignToProject();
    void onReturnFromProject();
    void onSendToRepair();
    
    // Слот для обработки выбора строки в таблице
    void onTableSelectionChanged() const;
    void onProjectSelectionChanged() const;
    
    // Слот для фильтрации по статусу
    void onStatusFilterChanged(int index) const;
    
    // Слот для контекстного меню
    void showContextMenu(const QPoint& pos);
    void showProjectContextMenu(const QPoint& pos);
    
    // Слот для контекстного меню на заголовке таблицы
    void showColumnHeaderMenu(const QPoint& pos);

private:
    /**
     * @brief Инициализация UI компонентов
     */
    void setupUI();
    
    /**
     * @brief Создание боковой панели навигации
     */
    void setupSidebar();
    
    /**
     * @brief Создание таблицы с техникой
     */
    void setupTable();

    /**
     * @brief Создание таблицы с проектами
     */
    void setupProjectsTable();
    
    /**
     * @brief Создание панели с деталями выбранной техники
     */
    void setupDetailsPanel();
    
    /**
     * @brief Подключение сигналов и слотов
     */
    void connectSignals();
    
    /**
     * @brief Обновление панели деталей
     * @param machine Указатель на выбранную технику
     */
    void updateDetailsPanel(const MachinePtr& machine) const;
    
    /**
     * @brief Обновление статусбара с количеством техники
     */
    void updateStatusBar() const;
    
    /**
     * @brief Получить выбранную технику из таблицы
     * @return Указатель на выбранную технику или nullptr
     */
    MachinePtr getSelectedMachine() const;

    /**
     * @brief Получить выбранный проект из таблицы
     * @return Указатель на выбранный проект или nullptr
     */
    ProjectPtr getSelectedProject() const;
    
    Ui::MainWindow *ui;
    
    QStackedWidget *m_stackedWidget;
    
    // Вид техники
    MachineTableModel *m_tableModel;
    QTableView *m_tableView;
    
    // Вид проектов
    ProjectTableModel *m_projectTableModel;
    QTableView *m_projectTableView;
    
    // Панель деталей
    QWidget *m_detailsPanel;
    QLabel *m_detailsName;
    QLabel *m_detailsType;
    QLabel *m_detailsSerial;
    QLabel *m_detailsYear;
    QLabel *m_detailsStatus;
    QLabel *m_detailsCost;
    QLabel *m_detailsProject;
    QLabel *m_detailsAssignedDate;
    
    // Фильтр по статусу
    QComboBox *m_statusFilter;

    // Кнопки навигации
    QPushButton *m_btnFleet;
    QPushButton *m_btnProjects;
};
