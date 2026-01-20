#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "../database/FleetDatabase.h"
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_usdToRubRate(77.7586)
    , m_rubToUsdRate(0.0129)
{
    ui->setupUi(this);
    setupUI();
    loadRates();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setupUI()
{
    setWindowTitle("Настройки");
    setModal(true);
    setMinimumWidth(400);
    
    // Set stylesheet for dark theme
    setStyleSheet(R"(
        QDialog {
            background-color: #1e1e1e;
        }
        QLabel {
            color: #d4d4d4;
        }
        QGroupBox {
            background-color: #252526;
            border: 1px solid #3e3e3e;
            border-radius: 4px;
            padding: 12px;
            color: #d4d4d4;
        }
        QDoubleSpinBox {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            border-radius: 2px;
            padding: 4px;
        }
        QPushButton {
            background-color: #094771;
            color: white;
            border: none;
            border-radius: 2px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #0e639c;
        }
        QPushButton:pressed {
            background-color: #063d56;
        }
    )");
}

void SettingsDialog::loadRates()
{
    // Load current rates from database
    m_usdToRubRate = FleetDatabase::instance().getCurrencyRate("USD", "RUB");
    m_rubToUsdRate = FleetDatabase::instance().getCurrencyRate("RUB", "USD");
    
    // Set values in spinboxes
    ui->usdToRubSpinBox->setValue(m_usdToRubRate);
    ui->rubToUsdSpinBox->setValue(m_rubToUsdRate);
}

bool SettingsDialog::validate()
{
    double usdRate = ui->usdToRubSpinBox->value();
    double rubRate = ui->rubToUsdSpinBox->value();
    
    if (usdRate <= 0.0 || rubRate <= 0.0) {
        QMessageBox::warning(this, "Ошибка", "Курсы обмена должны быть больше нуля");
        return false;
    }
    
    // Optional: Check if rates are reciprocals (within reasonable tolerance)
    double expectedRubToUsd = 1.0 / usdRate;
    double tolerance = 0.001;
    
    if (qAbs(rubRate - expectedRubToUsd) > tolerance) {
        auto reply = QMessageBox::question(this, "Внимание",
            QString("Курс RUB->USD не соответствует обратному от USD->RUB.\n"
                    "USD->RUB: %1\nОжидаемый RUB->USD: %2\nВаш RUB->USD: %3\n\n"
                    "Продолжить с этими значениями?")
            .arg(usdRate, 0, 'f', 4)
            .arg(expectedRubToUsd, 0, 'f', 4)
            .arg(rubRate, 0, 'f', 4),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) return false;
    }
    
    return true;
}

void SettingsDialog::accept()
{
    if (!validate()) {
        return;
    }
    
    double usdToRub = ui->usdToRubSpinBox->value();
    double rubToUsd = ui->rubToUsdSpinBox->value();
    
    // Save rates to database
    bool success = true;
    success = FleetDatabase::instance().setCurrencyRate("USD", "RUB", usdToRub) && success;
    success = FleetDatabase::instance().setCurrencyRate("RUB", "USD", rubToUsd) && success;
    
    if (!success) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить курсы валют");
        return;
    }
    
    QMessageBox::information(this, "Успешно", "Курсы валют сохранены");
    QDialog::accept();
}
