#include "SettingsPage.h"
#include "../StyleManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton> // <--- THIS WAS THE MISSING PIECE

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void SettingsPage::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(25);

    // 1. Page Title
    QLabel* title = new QLabel("System Settings");
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");
    mainLayout->addWidget(title);

    // 2. GENERAL SETTINGS SECTION
    QLabel* generalLabel = new QLabel("GENERAL");
    generalLabel->setStyleSheet("color: #56B6C2; font-weight: bold; letter-spacing: 2px;");
    mainLayout->addWidget(generalLabel);

    mainLayout->addWidget(createToggleSetting("Eco Mode", "Optimize energy consumption across all appliances."));
    mainLayout->addWidget(createToggleSetting("Push Notifications", "Receive alerts for security and climate changes."));
    mainLayout->addWidget(createToggleSetting("Dark Mode", "Toggle high-contrast dark theme (Always On)."));

    mainLayout->addSpacing(20);

    // 3. CONNECTIVITY SECTION
    QLabel* networkLabel = new QLabel("CONNECTIVITY");
    networkLabel->setStyleSheet("color: #56B6C2; font-weight: bold; letter-spacing: 2px;");
    mainLayout->addWidget(networkLabel);

    mainLayout->addWidget(createInputSetting("Server Gateway IP", "127.0.0.1"));
    mainLayout->addWidget(createInputSetting("Port", "8080"));

    // 4. Save Button (The part that was throwing errors)
    QPushButton* btnSave = new QPushButton("SAVE CHANGES");
    btnSave->setFixedWidth(200);
    btnSave->setStyleSheet(
        "background-color: #61AFEF; color: #12151A; font-weight: bold; padding: 12px; border-radius: 6px;"
    );
    mainLayout->addSpacing(30);
    mainLayout->addWidget(btnSave);

    mainLayout->addStretch();
}

QWidget* SettingsPage::createToggleSetting(QString title, QString description) {
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 10, 0, 10);

    QVBoxLayout* textStack = new QVBoxLayout();
    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");

    QLabel* lblDesc = new QLabel(description);
    lblDesc->setStyleSheet("color: #ABB2BF; font-size: 12px;");

    textStack->addWidget(lblTitle);
    textStack->addWidget(lblDesc);

    QCheckBox* toggle = new QCheckBox();
    // Simplified indicator style for now
    toggle->setStyleSheet("QCheckBox::indicator { width: 40px; height: 20px; }");

    layout->addLayout(textStack);
    layout->addStretch();
    layout->addWidget(toggle);

    row->setStyleSheet("border-bottom: 1px solid #2C313C;");
    return row;
}

QWidget* SettingsPage::createInputSetting(QString title, QString placeholder) {
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);

    QLabel* lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: white; min-width: 150px;");

    QLineEdit* input = new QLineEdit();
    input->setPlaceholderText(placeholder);
    input->setStyleSheet(StyleManager::getLoginInputStyle());
    input->setFixedWidth(250);

    layout->addWidget(lblTitle);
    layout->addStretch();
    layout->addWidget(input);

    return row;
}