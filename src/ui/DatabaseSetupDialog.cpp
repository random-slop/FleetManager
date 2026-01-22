#include "DatabaseSetupDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

DatabaseSetupDialog::DatabaseSetupDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Настройка базы данных");
    setMinimumWidth(400);

    auto* layout = new QVBoxLayout(this);

    auto* label = new QLabel("Файл базы данных не найден. Как вы хотите продолжить?", this);
    label->setWordWrap(true);
    layout->addWidget(label);

    auto* buttonLayout = new QHBoxLayout();
    
    auto* sampleBtn = new QPushButton("Создать демо-данные", this);
    auto* freshBtn = new QPushButton("Начать с нуля", this);
    auto* cancelBtn = new QPushButton("Отмена", this);

    buttonLayout->addWidget(sampleBtn);
    buttonLayout->addWidget(freshBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addLayout(buttonLayout);

    connect(sampleBtn, &QPushButton::clicked, this, [this]() { m_result = CreateSampleData; accept(); });
    connect(freshBtn, &QPushButton::clicked, this, [this]() { m_result = StartFresh; accept(); });
    connect(cancelBtn, &QPushButton::clicked, this, [this]() { m_result = Cancel; reject(); });
}

DatabaseSetupDialog::Result DatabaseSetupDialog::getSetupResult() const {
    return m_result;
}
