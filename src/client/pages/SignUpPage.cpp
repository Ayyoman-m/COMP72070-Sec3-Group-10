#include "SignUpPage.h"
#include "../StyleManager.h"

/**
 * @brief Constructs the SignUpPage.
 *
 * Initializes the user registration interface.
 */
SignUpPage::SignUpPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

/**
 * @brief Sets up the sign-up UI layout.
 *
 * Creates input fields for username, email, password,
 * and connects buttons for registration and navigation.
 */
void SignUpPage::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(350, 100, 350, 100);
    layout->setSpacing(15);

    QLabel* title = new QLabel("<h1>Create Pro Account</h1>");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #61AFEF; margin-bottom: 10px;");

    userEdit = new QLineEdit();
    userEdit->setPlaceholderText("New Username");
    userEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("Email Address");
    emailEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    passEdit = new QLineEdit();
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    confirmPassEdit = new QLineEdit();
    confirmPassEdit->setPlaceholderText("Confirm Password");
    confirmPassEdit->setEchoMode(QLineEdit::Password);
    confirmPassEdit->setStyleSheet(StyleManager::getLoginInputStyle());

    QPushButton* btnRegister = new QPushButton("REGISTER SYSTEM");
    btnRegister->setStyleSheet("background-color: #98C379; color: #12151A; font-weight: bold; padding: 12px; border-radius: 5px;");

    QPushButton* btnBack = new QPushButton("Back to Login");
    btnBack->setStyleSheet("background: transparent; color: #61AFEF; border: none; text-decoration: underline;");

    statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(userEdit);
    layout->addWidget(emailEdit);
    layout->addWidget(passEdit);
    layout->addWidget(confirmPassEdit);
    layout->addWidget(btnRegister);
    layout->addWidget(btnBack);
    layout->addWidget(statusLabel);
    layout->addStretch();

    connect(btnRegister, &QPushButton::clicked, this, &SignUpPage::onRegisterClicked);
    connect(btnBack, &QPushButton::clicked, this, &SignUpPage::backToLoginRequested);
}

/**
 * @brief Handles user registration logic.
 *
 * Validates input fields and emits registration signal
 * if data is valid.
 */
void SignUpPage::onRegisterClicked() {
    if (userEdit->text().isEmpty() || passEdit->text().isEmpty()) {
        statusLabel->setText("Username and Password are required.");
        statusLabel->setStyleSheet("color: #E06C75;");
        return;
    }

    if (passEdit->text() != confirmPassEdit->text()) {
        statusLabel->setText("Passwords do not match!");
        statusLabel->setStyleSheet("color: #E06C75;");
        return;
    }

    emit registrationRequested(userEdit->text(), passEdit->text(), emailEdit->text());

    statusLabel->setText("Account Created! You can now log in.");
    statusLabel->setStyleSheet("color: #98C379;");
}