#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>


/**
 * @class ProfilePage
 * @brief Displays user profile and account information.
 *
 * Shows user details such as name, email, and access level
 * in a structured layout.
 */
class ProfilePage : public QWidget {
    Q_OBJECT

public:

    /**
     * @brief Constructs the ProfilePage.
     * @param parent Parent widget
     */
    explicit ProfilePage(QWidget* parent = nullptr);

private:

    /**
    * @brief Initializes the UI layout.
    */
    void setupUi();

    /**
    * @brief Creates a row displaying user information.
    *
    * @param label Field name (e.g., Email)
    * @param value Field value
    * @return QWidget pointer to the created row
    */
    QWidget* createInfoRow(QString label, QString value);

    QLabel* titleLabel;
    QLabel* avatarPlaceholder;
};