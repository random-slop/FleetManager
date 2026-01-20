#include "MachineDialog.h"
#include "ui_MachineDialog.h"
#include "../database/FleetDatabase.h"
#include <QMessageBox>
#include <QDate>

MachineDialog::MachineDialog(QWidget *parent, const MachinePtr& machine)
    : QDialog(parent)
    , ui(new Ui::MachineDialog)
    , m_machine(machine)
    , m_isEditMode(machine != nullptr)
{
    ui->setupUi(this);
    setupUI();
    
    if (m_isEditMode) {
        fillFromMachine(machine);
    }
}

MachineDialog::~MachineDialog()
{
    delete ui;
}

void MachineDialog::setupUI()
{
    // Устанавливаем заголовок окна
    setWindowTitle(m_isEditMode ? "Редактировать технику" : "Добавить технику");
    
    // Устанавливаем текущий год по умолчанию
    ui->spinYear->setValue(QDate::currentDate().year());
    
    // Стилизация
    setStyleSheet(R"(
        QDialog {
            background-color: #2d2d2d;
        }
        QGroupBox {
            color: #d4d4d4;
            border: 1px solid #3e3e3e;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 12px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QLabel {
            color: #cccccc;
        }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            border-radius: 2px;
            padding: 4px;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 1px solid #007acc;
        }
        QPushButton {
            background-color: #0e639c;
            color: white;
            border: none;
            padding: 6px 20px;
            border-radius: 2px;
        }
        QPushButton:hover {
            background-color: #1177bb;
        }
        QPushButton:pressed {
            background-color: #094771;
        }
    )");
}

void MachineDialog::fillFromMachine(MachinePtr machine)
{
    if (!machine) return;
    
    ui->editName->setText(machine->getName());
    ui->comboType->setCurrentText(machine->getType());
    ui->editSerialNumber->setText(machine->getSerialNumber());
    ui->spinYear->setValue(machine->getYearOfManufacture());
    ui->spinCost->setValue(static_cast<int>(machine->getCost().getAmount()));
    ui->comboCurrency->setCurrentIndex(currencyToIndex(machine->getCost().getCurrency()));
    ui->comboStatus->setCurrentText(Machine::statusToString(machine->getStatus()));
}

bool MachineDialog::validate()
{
    // Проверка обязательных полей
    if (ui->editName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", "Название техники не может быть пустым");
        ui->editName->setFocus();
        return false;
    }
    
    if (ui->comboType->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", "Тип техники не может быть пустым");
        ui->comboType->setFocus();
        return false;
    }
    
    if (ui->editSerialNumber->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", "Серийный номер не может быть пустым");
        ui->editSerialNumber->setFocus();
        return false;
    }
    
    // Проверка уникальности серийного номера
    const QString serialNumber = ui->editSerialNumber->text().trimmed();
    if (!m_isEditMode || serialNumber != m_machine->getSerialNumber()) {
        if (!isSerialNumberUnique(serialNumber)) {
            QMessageBox::warning(this, "Ошибка валидации",
                               QString("Серийный номер \"%1\" уже используется.\n"
                                      "Пожалуйста, введите уникальный серийный номер.")
                               .arg(serialNumber));
            ui->editSerialNumber->selectAll();
            ui->editSerialNumber->setFocus();
            return false;
        }
    }
    
    // Проверка года выпуска
    const int currentYear = QDate::currentDate().year();
    if (ui->spinYear->value() > currentYear + 1) {
        QMessageBox::warning(this, "Ошибка валидации",
                           QString("Год выпуска не может быть больше %1").arg(currentYear + 1));
        ui->spinYear->setFocus();
        return false;
    }
    
    // Проверка стоимости
    if (ui->spinCost->value() <= 0) {
        QMessageBox::warning(this, "Ошибка валидации", "Стоимость должна быть больше нуля");
        ui->spinCost->setFocus();
        return false;
    }
    
    return true;
}

bool MachineDialog::isSerialNumberUnique(const QString& serialNumber)
{
    // Получаем все машины из БД
    auto allMachines = FleetDatabase::instance().getAllMachines();
    
    // Проверяем, существует ли машина с таким серийным номером
    for (const auto& machine : allMachines) {
        if (machine->getSerialNumber() == serialNumber) {
            // Если мы в режиме редактирования и это наша машина - ОК
            if (m_isEditMode && m_machine && machine->getId() == m_machine->getId()) {
                continue;
            }
            return false; // Найдена другая машина с таким номером
        }
    }
    
    return true; // Серийный номер уникален
}

void MachineDialog::accept()
{
    // Валидация перед закрытием
    if (!validate()) {
        return;
    }
    
    // Вызываем базовую реализацию для закрытия диалога
    QDialog::accept();
}

MachinePtr MachineDialog::getMachine() const
{
    // Создаём новую машину или обновляем существующую
    MachinePtr machine = m_isEditMode ? m_machine : std::make_shared<Machine>();
    
    machine->setName(ui->editName->text().trimmed());
    machine->setType(ui->comboType->currentText().trimmed());
    machine->setSerialNumber(ui->editSerialNumber->text().trimmed());
    machine->setYearOfManufacture(ui->spinYear->value());

    const Currency currency = indexToCurrency(ui->comboCurrency->currentIndex());
    machine->setCost(Money(ui->spinCost->value(), currency));
    
    machine->setStatus(Machine::stringToStatus(ui->comboStatus->currentText()));
    
    return machine;
}

Currency MachineDialog::indexToCurrency(const int index) const
{
    switch (index) {
        case 0: return Currency::RUB;
        case 1: return Currency::USD;
        default: return Currency::RUB;
    }
}

int MachineDialog::currencyToIndex(const Currency currency) const
{
    switch (currency) {
        case Currency::RUB: return 0;
        case Currency::USD: return 1;
        default: return 0;
    }
}
