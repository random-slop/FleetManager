#pragma once

#include <QString>
#include <QMap>

/**
 * @brief Поддерживаемые валюты
 */
enum class Currency {
    RUB, // Российский рубль
    USD  // Доллар США
};

/**
 * @brief Класс для работы с денежными суммами в разных валютах
 * 
 * Позволяет хранить сумму в определённой валюте, конвертировать
 * между валютами и сравнивать суммы.
 */
class Money {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Money();
    
    /**
     * @brief Конструктор с параметрами
     * @param amount Сумма
     * @param currency Валюта
     */
    Money(double amount, Currency currency = Currency::RUB);
    
    // Геттеры
    double getAmount() const { return m_amount; }
    Currency getCurrency() const { return m_currency; }
    
    // Сеттеры
    void setAmount(double amount) { m_amount = amount; }
    void setCurrency(Currency currency) { m_currency = currency; }
    
    /**
     * @brief Конвертировать в другую валюту
     * @param targetCurrency Целевая валюта
     * @return Новый объект Money в целевой валюте
     */
    Money convertTo(Currency targetCurrency) const;
    
    /**
     * @brief Получить сумму в рублях (для сравнения и сортировки)
     * @return Сумма в рублях
     */
    double toRubles() const;
    
    /**
     * @brief Форматированная строка с валютой
     * @return Строка вида "8 500 000 ₽" или "$100,000"
     */
    QString toString() const;
    
    /**
     * @brief Символ валюты
     * @param currency Валюта
     * @return Символ (₽, $, €, ¥)
     */
    static QString getCurrencySymbol(Currency currency);
    
    /**
     * @brief Название валюты
     * @param currency Валюта
     * @return Название (RUB, USD, EUR, CNY)
     */
    static QString getCurrencyName(Currency currency);
    
    /**
     * @brief Получить валюту по названию
     * @param name Название валюты
     * @return Enum валюты
     */
    static Currency currencyFromString(const QString& name);
    
    /**
     * @brief Получить курс обмена из БД
     * @param from Исходная валюта
     * @param to Целевая валюта
     * @return Курс обмена
     */
    static double getExchangeRate(Currency from, Currency to);
    
    // Операторы сравнения (сравнивают суммы в рублях)
    bool operator<(const Money& other) const;
    bool operator>(const Money& other) const;
    bool operator==(const Money& other) const;

private:
    double m_amount;
    Currency m_currency;
};
