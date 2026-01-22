#pragma once

#include <QDialog>

class DatabaseSetupDialog : public QDialog {
    Q_OBJECT
public:
    enum Result {
        CreateSampleData,
        StartFresh,
        Cancel
    };

    explicit DatabaseSetupDialog(QWidget* parent = nullptr);
    Result getSetupResult() const;

private:
    Result m_result = Cancel;
};
