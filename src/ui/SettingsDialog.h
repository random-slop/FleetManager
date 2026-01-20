#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

/**
 * @brief Диалог для редактирования настроек приложения
 * 
 * Позволяет пользователю установить курсы обмена валют USD/RUB и RUB/USD.
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

protected:
    void accept() override;

private:
    void setupUI();
    void loadRates();
    bool validate();
    
    Ui::SettingsDialog *ui;
    double m_usdToRubRate;
    double m_rubToUsdRate;
};
