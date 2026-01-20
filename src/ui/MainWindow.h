#pragma once

#include <QMainWindow>
#include "../models/Machine.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MachineTableModel;
class QTableView;
class QLabel;
class QVBoxLayout;
class QComboBox;

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
    // Слоты для действий меню и toolbar
    void onAddMachine();
    void onEditMachine();
    void onDeleteMachine();
    void onAssignToProject();
    void onReturnFromProject();
    void onSendToRepair();
    void onAbout();
    void onExit();
    
    // Слот для обработки выбора строки в таблице
    void onTableSelectionChanged();
    
    // Слот для фильтрации по статусу
    void onStatusFilterChanged(int index);

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
    void updateDetailsPanel(MachinePtr machine);
    
    /**
     * @brief Обновление статусбара с количеством техники
     */
    void updateStatusBar();
    
    /**
     * @brief Получить выбранную технику из таблицы
     * @return Указатель на выбранную технику или nullptr
     */
    MachinePtr getSelectedMachine();
    
    Ui::MainWindow *ui;
    MachineTableModel *m_tableModel;
    QTableView *m_tableView;
    
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
};
