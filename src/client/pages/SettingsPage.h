#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

private:
    void setupUi();

    // Helpers for different setting types
    QWidget* createToggleSetting(QString title, QString description);
    QWidget* createInputSetting(QString title, QString placeholder);

    QVBoxLayout* mainLayout;
};