#include "RoomDetailPage.h"
#include "../StyleManager.h"
#include <QHBoxLayout>
#include <QStyle>

RoomDetailPage::RoomDetailPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

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

void RoomDetailPage::loadRoom(const QString& roomName) {
    roomTitleLabel->setText(roomName.toUpper());
    clearLayout(deviceContainer);

    // Requirement: Every room gets a light + specialized controls
    if (roomName == "Living Room") {
        updateStatusBar("22.5°C", "ON");
        deviceContainer->insertWidget(0, createDeviceSwitch("Main Chandelier", true));

        // --- CLIMATE SYNC ---
        ClimateWidget* cw = new ClimateWidget();
        // This connection fixes the Temperature Header desync
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
        deviceContainer->insertWidget(2, createDeviceSlider("Ventilation Fan", 20, "%"));
    }
    else if (roomName == "Master Bedroom") {
        updateStatusBar("21.5°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Bedside Lamp", false));
        deviceContainer->insertWidget(1, createDeviceSwitch("Overhead Light", false));
        deviceContainer->insertWidget(2, createDeviceSlider("Mood Dimmer", 10, "%"));
    }
    else if (roomName == "Guest Room") {
        updateStatusBar("20.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Guest Light", false));
        deviceContainer->insertWidget(1, createDeviceSlider("Reading Lamp", 100, "%"));
    }
    else if (roomName == "Bathroom") {
        updateStatusBar("21.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Vanity Mirror Light", false));
        deviceContainer->insertWidget(1, createDeviceSwitch("Floor Heating", true));
    }
    else if (roomName == "Garage") {
        updateStatusBar("15.0°C", "OFF");
        deviceContainer->insertWidget(0, createDeviceSwitch("Garage High-Bay Lights", false));
        deviceContainer->insertWidget(1, createDeviceSwitch("Main Roller Door", false));
    }
    else if (roomName == "Backyard") {
        updateStatusBar("18.0°C", "DUSK");
        deviceContainer->insertWidget(0, createDeviceSwitch("Perimeter Flood Lights", false));
        deviceContainer->insertWidget(1, createDeviceSlider("Sprinkler Pressure", 0, " PSI"));
    }

    deviceContainer->addStretch();
}

// Fixed: Toggling the switch now updates the "LIGHTS: ON/OFF" header label
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

    // Lambda logic to toggle status AND update the parent header
    connect(btn, &QPushButton::clicked, [this, btn]() {
        bool currentlyOn = (btn->text() == "ON");
        if (currentlyOn) {
            btn->setText("OFF");
            btn->setObjectName("ToggleOff");
            roomLightStatusLabel->setText("LIGHTS: OFF"); // Fixes the desync
        }
        else {
            btn->setText("ON");
            btn->setObjectName("ToggleOn");
            roomLightStatusLabel->setText("LIGHTS: ON");  // Fixes the desync
        }
        // Style Refresh
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
        });

    layout->addWidget(btn);
    return card;
}

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

void RoomDetailPage::updateStatusBar(const QString& temp, const QString& lightStatus) {
    roomTempLabel->setText("TEMP: " + temp);
    roomLightStatusLabel->setText("LIGHTS: " + lightStatus);
}

void RoomDetailPage::clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) widget->deleteLater();
        delete item;
    }
}