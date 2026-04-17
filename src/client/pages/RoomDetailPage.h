#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QPixmap> // Added for the camera feed
#include "../components/ClimateWidget.h"

/**
 * @class RoomDetailPage
 * @brief Displays detailed view of a selected room and its devices.
 *
 * This class dynamically loads devices based on the selected room,
 * including lighting, climate controls, and security features.
 * It also supports receiving and displaying camera images.
 */
class RoomDetailPage : public QWidget {
    Q_OBJECT
    friend struct SmartHomeClientTestAccessor;

public:

    /**
     * @brief Constructs the RoomDetailPage.
     * @param parent Parent widget
     */
    explicit RoomDetailPage(QWidget* parent = nullptr);

    /**
    * @brief Loads and displays a room with its devices.
    * @param roomName Name of the selected room
    */
    void loadRoom(const QString& roomName);

    /**
     * @brief Updates camera display with received image.
     *
     * Implements REQ-SVR-070 (image transfer).
     *
     * @param pix Received image
     */
    void updateCameraDisplay(const QPixmap& pix);

signals:

    /**
     * @brief Emitted when the back button is pressed.
     */
    void backButtonClicked();

    /**
    * @brief Emitted when user requests a security image.
    *
    * Implements REQ-SVR-070.
    */
    void imageRequestTriggered();

private:

    /**
    * @brief Initializes UI layout.
    */
    void setupUi();

    /**
     * @brief Updates room status display.
     */
    void updateStatusBar(const QString& temp, const QString& lightStatus);

    // UI Helpers

    /**
     * @brief Creates a device toggle switch.
     */
    QWidget* createDeviceSwitch(QString name, bool isOn);

    /**
    * @brief Creates a device slider control.
    */
    QWidget* createDeviceSlider(QString name, int initialValue, QString unit);

    /**
    * @brief Sets up security camera UI.
    */
    void setupCameraView(QVBoxLayout* layout);

    /**
    * @brief Clears all widgets from layout.
    */
    void clearLayout(QLayout* layout);

    // Header Elements
    QLabel* roomTitleLabel;
    QLabel* roomTempLabel;
    QLabel* roomLightStatusLabel;

    // Main Container
    QVBoxLayout* deviceContainer;

    // Security Elements (REQ-SVR-070)
    QLabel* cameraMonitor;
    QPushButton* btnRequestImage;
};
