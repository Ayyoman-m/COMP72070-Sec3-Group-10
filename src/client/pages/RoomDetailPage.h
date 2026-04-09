#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QPixmap> // Added for the camera feed
#include "../components/ClimateWidget.h"

class RoomDetailPage : public QWidget {
    Q_OBJECT

public:
    explicit RoomDetailPage(QWidget* parent = nullptr);
    void loadRoom(const QString& roomName);

    // NEW: Needed so SmartHomeClient can send the received JPEG here
    void updateCameraDisplay(const QPixmap& pix);

signals:
    void backButtonClicked();
    void imageRequestTriggered();

private:
    void setupUi();
    void updateStatusBar(const QString& temp, const QString& lightStatus);

    // UI Generation Helpers
    QWidget* createDeviceSwitch(QString name, bool isOn);
    QWidget* createDeviceSlider(QString name, int initialValue, QString unit);
    void setupCameraView(QVBoxLayout* layout); // Added helper for cleaner code
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