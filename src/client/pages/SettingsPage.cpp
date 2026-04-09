#include "SettingsPage.h"
#include "../StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void SettingsPage::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);

    QLabel* title = new QLabel("System Settings");
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");
    mainLayout->addWidget(title);

    // --- GENERAL SECTION ---
    mainLayout->addWidget(createSectionHeader("GENERAL"));

    mainLayout->addWidget(createSettingRow(
        "Eco Mode",
        "Optimize energy consumption across all appliances"
    ));

    mainLayout->addWidget(createSettingRow(
        "Push Notifications",
        "Receive alerts for security and climate changes"
    ));

    mainLayout->addWidget(createSettingRow(
        "Dark Mode",
        "Toggle high-contrast dark theme (Always On)"
    ));

    // --- CONNECTIVITY SECTION ---
    mainLayout->addWidget(createSectionHeader("CONNECTIVITY"));

    // Server IP Row
    QWidget* ipRow = new QWidget();
    QHBoxLayout* ipLayout = new QHBoxLayout(ipRow);
    ipLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* ipLabel = new QLabel("Server Gateway IP");
    ipLabel->setStyleSheet("color: #ABB2BF; font-size: 14px;");
    QLineEdit* ipInput = new QLineEdit("127.0.0.1");
    ipInput->setFixedWidth(300);
    ipInput->setStyleSheet(StyleManager::getLoginInputStyle());
    ipLayout->addWidget(ipLabel);
    ipLayout->addStretch();
    ipLayout->addWidget(ipInput);
    mainLayout->addWidget(ipRow);

    // Port Row
    QWidget* portRow = new QWidget();
    QHBoxLayout* portLayout = new QHBoxLayout(portRow);
    portLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* portLabel = new QLabel("Port");
    portLabel->setStyleSheet("color: #ABB2BF; font-size: 14px;");
    QLineEdit* portInput = new QLineEdit("8080");
    portInput->setFixedWidth(300);
    portInput->setStyleSheet(StyleManager::getLoginInputStyle());
    portLayout->addWidget(portLabel);
    portLayout->addStretch();
    portLayout->addWidget(portInput);
    mainLayout->addWidget(portRow);

    mainLayout->addStretch();

    // Save Button
    QPushButton* btnSave = new QPushButton("SAVE CHANGES");
    btnSave->setFixedSize(200, 45);
    btnSave->setStyleSheet(
        "QPushButton { background-color: #61AFEF; color: #12151A; font-weight: bold; border-radius: 5px; }"
        "QPushButton:hover { background-color: #52a0e0; }"
    );
    mainLayout->addWidget(btnSave);
}

// Helper to create clean headers
QWidget* SettingsPage::createSectionHeader(QString title) {
    QLabel* header = new QLabel(title);
    header->setStyleSheet("color: #56B6C2; font-weight: bold; letter-spacing: 1.5px; margin-top: 10px;");
    return header;
}

// Helper to fix the "Squashed" text issue
QWidget* SettingsPage::createSettingRow(QString title, QString description) {
    QWidget* row = new QWidget();
    row->setFixedHeight(60); // Give it enough vertical room
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 5, 0, 5);

    // Vertical layout for text
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");

    QLabel* descLabel = new QLabel(description);
    descLabel->setStyleSheet("color: #5C6370; font-size: 12px;");
    descLabel->setWordWrap(true);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);

    layout->addLayout(textLayout);
    layout->addStretch();

    // The Toggle (You can use a QCheckBox or a custom QPushButton)
    QPushButton* toggle = new QPushButton("ON");
    toggle->setFixedSize(50, 26);
    toggle->setStyleSheet(
        "QPushButton { background: #2C313C; color: #ABB2BF; border: 1px solid #3E4451; border-radius: 13px; font-size: 10px; font-weight: bold; }"
    );

    layout->addWidget(toggle);

    // Add a subtle separator line at the bottom
    row->setStyleSheet("border-bottom: 1px solid #1E222A;");

    return row;
}