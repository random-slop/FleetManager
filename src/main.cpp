#include <QApplication>
#include "ui/MainWindow.h"
#include "database/FleetDatabase.h"
#include "ui/DatabaseSetupDialog.h"
#include <QDebug>
#include <QStyleFactory>
#include <QFile>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    QFont defaultFont("Segoe UI");

    defaultFont.setStyleStrategy(QFont::PreferAntialias);

    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    app.setStyleSheet(R"(
        QDialog { background-color: #1e1e1e; }
        QLabel { color: #d4d4d4; }
        QLineEdit, QSpinBox, QComboBox, QPlainTextEdit {
            background-color: #3c3c3c;
            color: #d4d4d4;
            border: 1px solid #555555;
            border-radius: 2px;
            padding: 4px;
        }
        QPushButton {
            background-color: #0e639c;
            color: white;
            min-width: 80px;
            padding: 6px;
        }
        QGroupBox {
            color: #858585;
            font-weight: bold;
        }
    )");
    
    const QString dbPath = "fleet.db";
    bool createSample = false;

    if (!QFile::exists(dbPath)) {
        DatabaseSetupDialog setupDialog;
        if (setupDialog.exec() == QDialog::Accepted) {
            if (setupDialog.getSetupResult() == DatabaseSetupDialog::CreateSampleData) {
                createSample = true;
            }
        } else return 0; // Пользователь закрыл окно или нажал Отмена
    }

    // Инициализация базы данных
    if (!FleetDatabase::instance().initialize(dbPath, createSample)) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось инициализировать базу данных!");
        return 1;
    }
    
    qDebug() << "База данных успешно инициализирована";
    
    // Создаём и показываем главное окно
    MainWindow window;
    window.show();

    return app.exec();
}
