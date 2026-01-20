#pragma once

#include <QDialog>
#include "../models/Machine.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MachineDialog; }
QT_END_NAMESPACE

/**
 * @brief Диалог для добавления или редактирования техники
 * 
 * Позволяет пользователю ввести все необходимые данные о технике
 * с валидацией и проверкой уникальности серийного номера.
 */
class MachineDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Конструктор диалога
     * @param parent Родительский виджет
     * @param machine Машина для редактирования (nullptr для создания новой)
     */
    explicit MachineDialog(QWidget *parent = nullptr, MachinePtr machine = nullptr);
    
    /**
     * @brief Деструктор
     */
    ~MachineDialog();
    
    /**
     * @brief Получить машину с введёнными данными
     * @return Указатель на объект Machine
     */
    MachinePtr getMachine() const;

protected:
    /**
     * @brief Переопределение accept() для валидации данных
     */
    void accept() override;

private:
    /**
     * @brief Инициализация UI
     */
    void setupUI();
    
    /**
     * @brief Заполнить форму данными машины (для редактирования)
     * @param machine Указатель на машину
     */
    void fillFromMachine(MachinePtr machine);
    
    /**
     * @brief Валидация введённых данных
     * @return true если все данные корректны, иначе false
     */
    bool validate();
    
    /**
     * @brief Проверить уникальность серийного номера
     * @param serialNumber Серийный номер для проверки
     * @return true если уникален, иначе false
     */
    bool isSerialNumberUnique(const QString& serialNumber);
    
    Ui::MachineDialog *ui;
    MachinePtr m_machine;           // Редактируемая машина (nullptr для новой)
    bool m_isEditMode;               // true если редактирование, false если создание
};
