#include <QApplication>
#include "ui/MainWindow.h"
#include "database/FleetDatabase.h"
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    // Устанавливаем стиль приложения
    app.setStyle("Fusion");
    
    // Инициализация базы данных
    if (!FleetDatabase::instance().initialize("fleet.db")) {
        qCritical() << "Не удалось инициализировать базу данных!";
        return 1;
    }
    
    qDebug() << "База данных успешно инициализирована";
    
    // Создаём и показываем главное окно
    MainWindow window;
    window.show();

    return app.exec();
}