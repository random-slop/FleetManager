#include "Money.h"
#include <QLocale>

// Инициализация статической переменной
QMap<QPair<Currency, Currency>, double> Money::s_exchangeRates;

Money::Money()
    : m_amount(0.0)
    , m_currency(Currency::RUB)
{
}

Money::Money(double amount, Currency currency)
    : m_amount(amount)
    , m_currency(currency)
{
}

Money Money::convertTo(Currency targetCurrency) const
{
    if (m_currency == targetCurrency) {
        return *this; // Та же валюта, возвращаем копию
    }
    
    double rate = getExchangeRate(m_currency, targetCurrency);
    return Money(m_amount * rate, targetCurrency);
}

double Money::toRubles() const
{
    if (m_currency == Currency::RUB) {
        return m_amount;
    }
    
    double rate = getExchangeRate(m_currency, Currency::RUB);
    return m_amount * rate;
}

QString Money::toString() const
{
    QLocale locale(QLocale::Russian);
    QString symbol = getCurrencySymbol(m_currency);
    
    // Форматируем с разделителями тысяч
    QString formattedAmount = locale.toString(m_amount, 'f', 0);
    
    // Для рублей и юаней символ после числа, для долларов и евро — перед
    if (m_currency == Currency::USD || m_currency == Currency::EUR) {
        return symbol + formattedAmount;
    } else {
        return formattedAmount + " " + symbol;
    }
}

QString Money::getCurrencySymbol(Currency currency)
{
    switch (currency) {
        case Currency::RUB: return "₽";
        case Currency::USD: return "$";
        case Currency::EUR: return "€";
        case Currency::CNY: return "¥";
        default: return "?";
    }
}

QString Money::getCurrencyName(Currency currency)
{
    switch (currency) {
        case Currency::RUB: return "RUB";
        case Currency::USD: return "USD";
        case Currency::EUR: return "EUR";
        case Currency::CNY: return "CNY";
        default: return "UNKNOWN";
    }
}

Currency Money::currencyFromString(const QString& name)
{
    if (name == "RUB") return Currency::RUB;
    if (name == "USD") return Currency::USD;
    if (name == "EUR") return Currency::EUR;
    if (name == "CNY") return Currency::CNY;
    return Currency::RUB; // По умолчанию
}

void Money::setExchangeRate(Currency from, Currency to, double rate)
{
    s_exchangeRates[qMakePair(from, to)] = rate;
    // Обратный курс
    if (rate > 0) {
        s_exchangeRates[qMakePair(to, from)] = 1.0 / rate;
    }
}

double Money::getExchangeRate(Currency from, Currency to)
{
    if (from == to) {
        return 1.0;
    }
    
    auto key = qMakePair(from, to);
    if (s_exchangeRates.contains(key)) {
        return s_exchangeRates[key];
    }
    
    // Если курса нет, возвращаем 1 (предполагаем равенство)
    return 1.0;
}

void Money::initializeExchangeRates()
{
    // Курсы валют на январь 2026 (приблизительные)
    // Базовая валюта: RUB
    
    // RUB <-> USD
    setExchangeRate(Currency::RUB, Currency::USD, 0.0105); // 1 RUB = 0.0105 USD (курс ~95 руб за доллар)
    
    // RUB <-> EUR
    setExchangeRate(Currency::RUB, Currency::EUR, 0.0095); // 1 RUB = 0.0095 EUR (курс ~105 руб за евро)
    
    // RUB <-> CNY
    setExchangeRate(Currency::RUB, Currency::CNY, 0.075);  // 1 RUB = 0.075 CNY (курс ~13.3 руб за юань)
    
    // USD <-> EUR
    setExchangeRate(Currency::USD, Currency::EUR, 0.91);   // 1 USD = 0.91 EUR
    
    // USD <-> CNY
    setExchangeRate(Currency::USD, Currency::CNY, 7.15);   // 1 USD = 7.15 CNY
    
    // EUR <-> CNY
    setExchangeRate(Currency::EUR, Currency::CNY, 7.85);   // 1 EUR = 7.85 CNY
}

bool Money::operator<(const Money& other) const
{
    return toRubles() < other.toRubles();
}

bool Money::operator>(const Money& other) const
{
    return toRubles() > other.toRubles();
}

bool Money::operator==(const Money& other) const
{
    // Сравниваем с погрешностью 0.01 рубля
    return qAbs(toRubles() - other.toRubles()) < 0.01;
}
