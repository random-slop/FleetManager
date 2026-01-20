#include "Money.h"
#include <QLocale>
#include "../database/FleetDatabase.h"

Money::Money(): m_amount(0.0), m_currency(Currency::RUB) {}

Money::Money(const double amount, const Currency currency): m_amount(amount), m_currency(currency) {}

Money Money::convertTo(const Currency targetCurrency) const
{
    if (m_currency == targetCurrency) return *this;

    const double rate = getExchangeRate(m_currency, targetCurrency);
    return Money(m_amount * rate, targetCurrency);
}

double Money::toRubles() const
{
    if (m_currency == Currency::RUB) return m_amount;

    const double rate = getExchangeRate(m_currency, Currency::RUB);
    return m_amount * rate;
}

QString Money::toString() const
{
    const QLocale locale(QLocale::Russian);
    const QString symbol = getCurrencySymbol(m_currency);
    
    const QString formattedAmount = locale.toString(m_amount, 'f', 0);

    if (m_currency == Currency::USD) return symbol + formattedAmount;
    return formattedAmount + " " + symbol;
}

QString Money::getCurrencySymbol(const Currency currency)
{
    switch (currency) {
        case Currency::RUB: return "₽";
        case Currency::USD: return "$";
        default: return "?";
    }
}

QString Money::getCurrencyName(const Currency currency)
{
    switch (currency) {
        case Currency::RUB: return "RUB";
        case Currency::USD: return "USD";
        default: return "UNKNOWN";
    }
}

Currency Money::currencyFromString(const QString& name)
{
    if (name == "RUB") return Currency::RUB;
    if (name == "USD") return Currency::USD;
    return Currency::RUB; // По умолчанию
}

double Money::getExchangeRate(Currency from, Currency to)
{
    if (from == to) return 1.0;

    // Получаем курс из базы данных
    QString fromStr = getCurrencyName(from);
    QString toStr = getCurrencyName(to);
    return FleetDatabase::instance().getCurrencyRate(fromStr, toStr);
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
