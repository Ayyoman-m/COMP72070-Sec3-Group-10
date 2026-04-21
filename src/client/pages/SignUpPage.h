#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @class SignUpPage
 * @brief User registration interface for the Smart Home system.
 *
 * Allows users to create a new account by entering username,
 * email, and password, with basic validation.
 */
class SignUpPage : public QWidget {
    Q_OBJECT

public:

    /**
     * @brief Constructs the SignUpPage.
     * @param parent Parent widget
     */
    explicit SignUpPage(QWidget* parent = nullptr);

signals:

    /**
     * @brief Emitted when a new user registers.
     *
     * Sends registration data to SmartHomeClient for storage.
     *
     * @param user Username
     * @param pass Password
     * @param email Email address
     */
    void registrationRequested(QString user, QString pass, QString email);

    /**
     * @brief Emitted when user requests to return to login page.
     */
    void backToLoginRequested();

private slots:

    /**
     * @brief Handles register button click.
     *
     * Validates input and emits registration signal if valid.
     */
    void onRegisterClicked();

private:

    /**
     * @brief Initializes UI layout.
     */
    void setupUi();

    QLineEdit* userEdit;
    QLineEdit* emailEdit;
    QLineEdit* passEdit;
    QLineEdit* confirmPassEdit;
    QLabel* statusLabel;
};