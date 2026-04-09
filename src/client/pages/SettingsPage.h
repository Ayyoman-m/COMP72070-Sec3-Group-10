#pragma once

#include <QWidget>
#include <QString>

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

private:
    void setupUi();

    // These declarations are the "missing members" the compiler is looking for:
    QWidget* createSectionHeader(QString title);
    QWidget* createSettingRow(QString title, QString description);
};