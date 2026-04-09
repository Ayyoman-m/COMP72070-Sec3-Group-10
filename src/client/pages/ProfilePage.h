#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class ProfilePage : public QWidget {
    Q_OBJECT

public:
    explicit ProfilePage(QWidget* parent = nullptr);

private:
    void setupUi();

    // Helper to create the "Info Rows" like Email, Role, etc.
    QWidget* createInfoRow(QString label, QString value);

    QLabel* titleLabel;
    QLabel* avatarPlaceholder;
};