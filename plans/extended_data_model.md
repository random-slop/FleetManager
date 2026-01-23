# План по расширению модели данных FleetManager

## 1. Введение

Цель: Расширить модель данных техники, добавив дополнительные поля для большей реалистичности. Выбранные поля:
- Пробег (mileage)
- Дата следующего обслуживания (next_maintenance_date)
- Дата покупки (purchase_date)
- Гарантийный срок (warranty_period)

## 2. Текущая структура

### 2.1. Таблица `machines` в базе данных

```sql
CREATE TABLE IF NOT EXISTS machines (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    serial_number TEXT UNIQUE NOT NULL,
    year_of_manufacture INTEGER NOT NULL,
    status TEXT NOT NULL,
    cost REAL NOT NULL,
    currency TEXT NOT NULL DEFAULT 'RUB',
    current_project TEXT,
    assigned_date TEXT
)
```

### 2.2. Класс Machine

```cpp
class Machine {
    int m_id;
    QString m_name;
    QString m_type;
    QString m_serialNumber;
    int m_yearOfManufacture;
    MachineStatus m_status;
    Money m_cost;
    QString m_currentProject;
    QDate m_assignedDate;
};
```

## 3. План изменений

### 3.1. Изменения в базе данных

Добавить новые поля в таблицу `machines`:

```sql
ALTER TABLE machines ADD COLUMN mileage INTEGER DEFAULT 0;
ALTER TABLE machines ADD COLUMN next_maintenance_date TEXT;
ALTER TABLE machines ADD COLUMN purchase_date TEXT;
ALTER TABLE machines ADD COLUMN warranty_period TEXT;
```

### 3.2. Изменения в классе Machine

Добавить новые поля в класс `Machine`:

```cpp
class Machine {
    // ... существующие поля ...
    int m_mileage;                      // Пробег
    QDate m_nextMaintenanceDate;        // Дата следующего обслуживания
    QDate m_purchaseDate;               // Дата покупки
    QString m_warrantyPeriod;           // Гарантийный срок
};
```

Добавить соответствующие геттеры и сеттеры:

```cpp
// Геттеры
int getMileage() const { return m_mileage; }
QDate getNextMaintenanceDate() const { return m_nextMaintenanceDate; }
QDate getPurchaseDate() const { return m_purchaseDate; }
QString getWarrantyPeriod() const { return m_warrantyPeriod; }

// Сеттеры
void setMileage(int mileage) { m_mileage = mileage; }
void setNextMaintenanceDate(const QDate& date) { m_nextMaintenanceDate = date; }
void setPurchaseDate(const QDate& date) { m_purchaseDate = date; }
void setWarrantyPeriod(const QString& period) { m_warrantyPeriod = period; }
```

### 3.3. Изменения в интерфейсе

#### 3.3.1. MachineDialog

Добавить новые поля в диалоговое окно:
- Поле для ввода пробега (QSpinBox)
- Поле для выбора даты следующего обслуживания (QDateEdit)
- Поле для выбора даты покупки (QDateEdit)
- Поле для ввода гарантийного срока (QLineEdit)

Обновить методы:
- `fillFromMachine()`: Заполнение новых полей
- `validate()`: Валидация новых полей
- `getMachine()`: Получение значений новых полей

#### 3.3.2. MachineTableModel

Добавить новые колонки в таблицу:
- Пробег
- Дата следующего обслуживания
- Дата покупки
- Гарантийный срок

Обновить методы:
- `data()`: Отображение данных новых полей
- `headerData()`: Заголовки новых колонок

## 4. Последовательность выполнения

1. **Обновить структуру базы данных**
   - Добавить новые поля в таблицу `machines`

2. **Обновить класс Machine**
   - Добавить новые поля
   - Добавить геттеры и сеттеры

3. **Обновить интерфейс**
   - Добавить новые поля в `MachineDialog`
   - Добавить новые колонки в `MachineTableModel`

## 5. Ожидаемые результаты

После внесения изменений:
- Модель данных техники станет более реалистичной
- Пользователи смогут вводить и просматривать дополнительную информацию о технике
- Интерфейс будет поддерживать новые поля без потери функциональности

## 6. Риски и ограничения

- Необходимо обеспечить совместимость с существующими данными
- Изменения в базе данных требуют миграции
- Новые поля должны быть опциональными для существующих записей