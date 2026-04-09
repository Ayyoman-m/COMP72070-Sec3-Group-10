#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class SignUpPage : public QWidget {
    Q_OBJECT

public:
    explicit SignUpPage(QWidget* parent = nullptr);

signals:
    // This sends the data back to SmartHomeClient to store in the vector
    void registrationRequested(QString user, QString pass, QString email);
    void backToLoginRequested();

private slots:
    void onRegisterClicked();

private:
    void setupUi();

    QLineEdit* userEdit;
    QLineEdit* emailEdit;
    QLineEdit* passEdit;
    QLineEdit* confirmPassEdit;
    QLabel* statusLabel;
};