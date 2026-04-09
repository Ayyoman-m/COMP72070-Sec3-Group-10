#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ClimateWidget : public QWidget {
    Q_OBJECT

public:
    explicit ClimateWidget(QWidget* parent = nullptr);

    // Public getters/setters for the system to interact with
    float getTargetTemp() const { return targetTemp; }
    QString getMode() const { return currentMode; }

signals:
    void targetTempChanged(float newTemp);
    void modeChanged(QString newMode);

private slots:
    void onSliderMoved(int value);
    void onModeSelected(int index);

private:
    void updateUI(); // Handles the neon colors and label text

    // UI Elements
    QLabel* titleLabel;
    QLabel* tempDisplay;
    QSlider* tempSlider;
    QComboBox* modeSelector;

    // Logic State
    float targetTemp;
    QString currentMode; // "COOL", "HEAT", "OFF"
};