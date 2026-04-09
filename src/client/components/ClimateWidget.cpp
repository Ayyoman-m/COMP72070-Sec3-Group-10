#include "ClimateWidget.h"

ClimateWidget::ClimateWidget(QWidget* parent)
    : QWidget(parent), targetTemp(22.0f), currentMode("OFF")
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 1. Header & Mode Selection
    QHBoxLayout* headerLayout = new QHBoxLayout();
    titleLabel = new QLabel("CLIMATE CONTROL");
    titleLabel->setStyleSheet("font-weight: bold; color: #61AFEF; letter-spacing: 1px;");

    modeSelector = new QComboBox();
    modeSelector->addItems({ "OFF", "COOL", "HEAT" });
    modeSelector->setFixedWidth(80);

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(modeSelector);
    mainLayout->addLayout(headerLayout);

    // 2. Large Temperature Display
    tempDisplay = new QLabel("22.0°C");
    tempDisplay->setAlignment(Qt::AlignCenter);
    tempDisplay->setStyleSheet("font-size: 48px; font-family: 'Consolas'; color: #ABB2BF; font-weight: bold;");
    mainLayout->addWidget(tempDisplay);

    // 3. The Interactive Slider
    tempSlider = new QSlider(Qt::Horizontal);
    tempSlider->setRange(160, 300); // 16.0°C to 30.0°C (multiplied by 10 for precision)
    tempSlider->setValue(220);
    tempSlider->setEnabled(false); // Disabled by default until mode is selected
    mainLayout->addWidget(tempSlider);

    // Connections
    connect(tempSlider, &QSlider::valueChanged, this, &ClimateWidget::onSliderMoved);
    connect(modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClimateWidget::onModeSelected);

    // Styling the widget container
    this->setObjectName("ClimateCard");
    this->setStyleSheet(
        "QWidget#ClimateCard { background-color: #161B22; border: 2px solid #2C313C; border-radius: 12px; }"
        "QSlider::groove:horizontal { border: 1px solid #3E4451; height: 8px; background: #1E222A; border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #61AFEF; border: 1px solid #61AFEF; width: 18px; margin: -5px 0; border-radius: 9px; }"
    );
}

void ClimateWidget::onModeSelected(int index) {
    currentMode = modeSelector->itemText(index);

    if (currentMode == "OFF") {
        tempSlider->setEnabled(false);
    }
    else {
        tempSlider->setEnabled(true);
        // Force bounds check immediately upon switching modes
        onSliderMoved(tempSlider->value());
    }
    updateUI();
    emit modeChanged(currentMode);
}

void ClimateWidget::onSliderMoved(int value) {
    float newTemp = value / 10.0f;

    // --- YOUR LOGIC GATES ---
    if (currentMode == "COOL" && newTemp > 24.0f) {
        newTemp = 24.0f;
        tempSlider->setValue(240);
    }
    else if (currentMode == "HEAT" && newTemp < 18.0f) {
        newTemp = 18.0f;
        tempSlider->setValue(180);
    }

    targetTemp = newTemp;
    updateUI();
    emit targetTempChanged(targetTemp);
}

void ClimateWidget::updateUI() {
    tempDisplay->setText(QString::number(targetTemp, 'f', 1) + "°C");

    // Dynamic Glow Colors
    if (currentMode == "OFF") {
        tempDisplay->setStyleSheet("font-size: 48px; color: #3E4451;");
        tempSlider->setStyleSheet("QSlider::handle:horizontal { background: #3E4451; }");
    }
    else if (currentMode == "COOL") {
        tempDisplay->setStyleSheet("font-size: 48px; color: #61AFEF; font-weight: bold;"); // Neon Blue
        tempSlider->setStyleSheet("QSlider::handle:horizontal { background: #61AFEF; }");
    }
    else if (currentMode == "HEAT") {
        tempDisplay->setStyleSheet("font-size: 48px; color: #E06C75; font-weight: bold;"); // Neon Red
        tempSlider->setStyleSheet("QSlider::handle:horizontal { background: #E06C75; }");
    }
}