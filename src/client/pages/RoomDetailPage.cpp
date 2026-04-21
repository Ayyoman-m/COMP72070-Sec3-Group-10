#include "RoomDetailPage.h"
#include "../StyleManager.h"
#include <QHBoxLayout>
#include <QStyle>

/**
 * @brief Constructs the RoomDetailPage.
 *
 * Initializes UI layout for displaying room devices and status.
 */
RoomDetailPage::RoomDetailPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

/**
 * @brief Sets up the room detail UI layout.
 *
 * Creates the status bar and device display area.
 */
void RoomDetailPage::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // --- STATUS BAR ---
    QWidget* statusBar = new QWidget();
    statusBar->setObjectName("StatusBar");
    statusBar->setFixedHeight(60);
    statusBar->setStyleSheet(StyleManager::getStatusBarStyle());

    QHBoxLayout* sLayout = new QHBoxLayout(statusBar);

    QPushButton* btnBack = new QPushButton("←");
    btnBack->setFixedSize(40, 40);
    btnBack->setStyleSheet("background: #2C313C; color: #61AFEF; border-radius: 20px; font-weight: bold;");
    connect(btnBack, &QPushButton::clicked, this, &RoomDetailPage::backButtonClicked);

    roomTitleLabel = new QLabel("ROOM");
    roomTempLabel = new QLabel("TEMP: --");
    roomLightStatusLabel = new QLabel("LIGHTS: --");

    sLayout->addWidget(btnBack);
    sLayout->addWidget(roomTitleLabel);
    sLayout->addStretch();
    sLayout->addWidget(roomTempLabel);
    sLayout->addSpacing(20);
    sLayout->addWidget(roomLightStatusLabel);
    mainLayout->addWidget(statusBar);

    // --- DEVICE AREA ---
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget* container = new QWidget();
    deviceContainer = new QVBoxLayout(container);
    deviceContainer->setSpacing(15);
    deviceContainer->addStretch();

    scroll->setWidget(container);
    mainLayout->addWidget(scroll);
}

/**
 * @brief Loads and displays devices for a selected room.
 *
 * Dynamically updates UI components based on room type,
 * including lighting, climate controls, and security features.
 *
 * Implements REQ-SVR-070 for security-enabled rooms.
 *
 * @param roomName Name of the selected room
 */
void RoomDetailPage::loadRoom(const QString& roomName) {
    roomTitleLabel->setText(roomName.toUpper());
    clearLayout(deviceContainer);

    bool hasSecurity = (roomName == "Garage" || roomName == "Backyard");

    if (roomName == "Living Room") {
        updateStatusBar("22.5°C", "ON");
        deviceContainer->insertWidget(0, createDeviceSwitch("Main Chandelier", true));
        ClimateWidget* cw = new ClimateWidget();
        connect(cw, &ClimateWidget::targetTempChanged, [this](float newTemp) {
            roomTempLabel->setText("TEMP: " + QString::number(newTemp, 'f', 1) + "°C");
            });
        deviceContainer->insertWidget(1, cw);
        deviceContainer->insertWidget(2, createDeviceSlider("Ceiling Fan Speed", 50, "%"));
    }
    else if (roomName == "Kitchen") {
        updateStatusBar("24.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Kitchen Fluorescents", false));
        deviceContainer->insertWidget(1, createDeviceSwitch("Smart Oven", false));
    }
    else if (roomName == "Master Bedroom") {
        updateStatusBar("21.5°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Overhead Light", false));
        deviceContainer->insertWidget(1, createDeviceSlider("Mood Dimmer", 10, "%"));
    }
    else if (roomName == "Garage") {
        updateStatusBar("15.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Main Roller Door", false));
        setupCameraView(deviceContainer); 
    }
    else if (roomName == "Backyard") {
        updateStatusBar("18.0°C", "DUSK");
        deviceContainer->insertWidget(0, createDeviceSwitch("Flood Lights", false));
        setupCameraView(deviceContainer); 
    }
    else {
        // Default catch-all for other rooms
        updateStatusBar("20.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("General Lighting", false));
    }

    deviceContainer->addStretch();
}

/**
 * @brief Sets up the security camera view UI.
 *
 * Adds camera display and request button for high-resolution images.
 *
 * @param layout Layout to insert camera components into
 */
void RoomDetailPage::setupCameraView(QVBoxLayout* layout) {
    QLabel* camTitle = new QLabel("LIVE SECURITY FEED (1MB SNAPSHOT)");
    camTitle->setStyleSheet("color: #56B6C2; font-weight: bold; margin-top: 20px;");
    layout->addWidget(camTitle);

    cameraMonitor = new QLabel("NO SIGNAL - CLICK TO REQUEST");
    cameraMonitor->setFixedSize(640, 360);
    cameraMonitor->setAlignment(Qt::AlignCenter);
    cameraMonitor->setStyleSheet("background: #000000; border: 2px solid #2C313C; color: #5C6370; font-family: monospace;");
    layout->addWidget(cameraMonitor);

    btnRequestImage = new QPushButton("CAPTURE HIGH-RES JPEG");
    btnRequestImage->setStyleSheet(
        "QPushButton { background: #61AFEF; color: #12151A; font-weight: bold; padding: 12px; border-radius: 4px; }"
        "QPushButton:hover { background: #52a0e0; }"
    );
    connect(btnRequestImage, &QPushButton::clicked, this, &RoomDetailPage::imageRequestTriggered);
    layout->addWidget(btnRequestImage);
}

/**
 * @brief Updates camera display with received image.
 *
 * Displays the image received from the server in the UI.
 *
 * Implements REQ-SVR-070 (image transfer).
 */
void RoomDetailPage::updateCameraDisplay(const QPixmap& pix) {
    if (cameraMonitor) {
        cameraMonitor->setPixmap(pix.scaled(cameraMonitor->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        cameraMonitor->setText(""); // Remove the "No Signal" text
    }
}

/**
 * @brief Creates a toggle switch for a device.
 *
 * @param name Device name
 * @param isOn Initial state
 * @return QWidget pointer to the created switch
 */
QWidget* RoomDetailPage::createDeviceSwitch(QString name, bool isOn) {
    QFrame* card = new QFrame();
    card->setObjectName("DeviceCard");
    card->setFixedHeight(70);
    card->setStyleSheet(StyleManager::getDeviceControlStyle());

    QHBoxLayout* layout = new QHBoxLayout(card);
    layout->addWidget(new QLabel("<b>" + name + "</b>"));
    layout->addStretch();

    QPushButton* btn = new QPushButton(isOn ? "ON" : "OFF");
    btn->setObjectName(isOn ? "ToggleOn" : "ToggleOff");
    btn->setFixedSize(80, 40);

    connect(btn, &QPushButton::clicked, [this, btn]() {
        bool currentlyOn = (btn->text() == "ON");
        btn->setText(currentlyOn ? "OFF" : "ON");
        btn->setObjectName(currentlyOn ? "ToggleOff" : "ToggleOn");
        roomLightStatusLabel->setText("LIGHTS: " + btn->text());

        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        });

    layout->addWidget(btn);
    return card;
}

/**
 * @brief Creates a slider control for a device.
 *
 * @param name Device name
 * @param initialValue Starting value
 * @param unit Unit of measurement
 * @return QWidget pointer to the created slider
 */
QWidget* RoomDetailPage::createDeviceSlider(QString name, int initialValue, QString unit) {
    QFrame* card = new QFrame();
    card->setObjectName("DeviceCard");
    card->setFixedHeight(100);
    card->setStyleSheet(StyleManager::getDeviceControlStyle());

    QVBoxLayout* layout = new QVBoxLayout(card);
    QHBoxLayout* top = new QHBoxLayout();

    QLabel* valLabel = new QLabel(QString::number(initialValue) + unit);
    valLabel->setStyleSheet("color: #61AFEF; font-weight: bold;");

    top->addWidget(new QLabel("<b>" + name + "</b>"));
    top->addStretch();
    top->addWidget(valLabel);

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(initialValue);

    connect(slider, &QSlider::valueChanged, [valLabel, unit](int value) {
        valLabel->setText(QString::number(value) + unit);
        });

    layout->addLayout(top);
    layout->addWidget(slider);
    return card;
}

/**
 * @brief Updates room status display.
 *
 * @param temp Temperature value
 * @param lightStatus Light status text
 */
void RoomDetailPage::updateStatusBar(const QString& temp, const QString& lightStatus) {
    roomTempLabel->setText("TEMP: " + temp);
    roomLightStatusLabel->setText("LIGHTS: " + lightStatus);
}

/**
 * @brief Clears all widgets from a layout.
 *
 * @param layout Layout to clear
 */
void RoomDetailPage::clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) widget->deleteLater();
        delete item;
    }
}