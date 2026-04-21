#pragma once

#include <QWidget>
#include <QString>

/**
 * @class SettingsPage
 * @brief Provides system configuration and user settings interface.
 *
 * Allows users to configure general preferences and network
 * settings such as server IP and port.
 */
class SettingsPage : public QWidget {
    Q_OBJECT

public:

    /**
     * @brief Constructs the SettingsPage.
     * @param parent Parent widget
     */
    explicit SettingsPage(QWidget* parent = nullptr);

private:

    /**
     * @brief Initializes the settings UI layout.
     */
    void setupUi();

    /**
     * @brief Creates a section header.
     *
     * @param title Section title
     * @return QWidget pointer to the header
     */
    QWidget* createSectionHeader(QString title);

    /**
     * @brief Creates a setting row with toggle.
     *
     * @param title Setting name
     * @param description Setting description
     * @return QWidget pointer to the created row
     */
    QWidget* createSettingRow(QString title, QString description);
};