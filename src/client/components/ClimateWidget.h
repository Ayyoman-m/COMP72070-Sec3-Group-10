#pragma once

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

/**
 * @class ClimateWidget
 * @brief UI component for controlling temperature and climate mode.
 *
 * This widget allows the user to adjust the target temperature
 * and select the operating mode (e.g., COOL, HEAT, OFF).
 * It emits signals when values change.
 */
class ClimateWidget : public QWidget {
    Q_OBJECT

/**
     * @brief Constructs the ClimateWidget.
     * @param parent Parent widget
     */
public:
    explicit ClimateWidget(QWidget* parent = nullptr);

    /**
    * @brief Returns the selected target temperature.
    * @return Target temperature value
    */
    float getTargetTemp() const { return targetTemp; }

    /**
    * @brief Returns the selected climate mode.
    * @return Current mode as a string
    */
    QString getMode() const { return currentMode; }

signals:

    /**
     * @brief Emitted when the target temperature changes.
     * @param newTemp Updated temperature value
     */
    void targetTempChanged(float newTemp);

    /**
  * @brief Emitted when the climate mode changes.
  * @param newMode Selected mode
  */
    void modeChanged(QString newMode);

private slots:

    /**
     * @brief Handles slider movement for temperature adjustment.
     * @param value Slider value
     */
    void onSliderMoved(int value);

    /**
     * @brief Handles mode selection changes.
     * @param index Selected mode index
     */
    void onModeSelected(int index);

private:

    /**
     * @brief Updates UI elements based on current state.
     *
     * Refreshes labels and styling based on temperature and mode.
     */
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