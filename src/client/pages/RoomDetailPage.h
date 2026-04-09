#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include "../components/ClimateWidget.h"

class RoomDetailPage : public QWidget {
    Q_OBJECT

public:
    explicit RoomDetailPage(QWidget* parent = nullptr);
    void loadRoom(const QString& roomName); // The dynamic loader

signals:
    void backButtonClicked();

private:
    void setupUi();
    void updateStatusBar(const QString& temp, const QString& lightStatus);

    // UI Generation Helpers (Requirements #2 & #3)
    QWidget* createDeviceSwitch(QString name, bool isOn);
    QWidget* createDeviceSlider(QString name, int initialValue, QString unit);
    void clearLayout(QLayout* layout);

    // Header Elements
    QLabel* roomTitleLabel;
    QLabel* roomTempLabel;
    QLabel* roomLightStatusLabel;

    // Main Container
    QVBoxLayout* deviceContainer;
};