#include "ProfilePage.h"
#include "../StyleManager.h"
#include <QHBoxLayout>
#include <QDateTime>

ProfilePage::ProfilePage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void ProfilePage::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);

    // 1. Profile Header Section
    QHBoxLayout* headerLayout = new QHBoxLayout();

    // This will eventually hold your User Icon
    avatarPlaceholder = new QLabel();
    avatarPlaceholder->setFixedSize(100, 100);
    avatarPlaceholder->setStyleSheet(
        "background-color: #1E222A; "
        "border: 2px solid #61AFEF; "
        "border-radius: 50px; " // Perfect circle
        "color: #61AFEF; "
        "font-size: 40px;"
    );
    avatarPlaceholder->setText("A"); // Default for Admin
    avatarPlaceholder->setAlignment(Qt::AlignCenter);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLabel = new QLabel("Administrator");
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");

    QLabel* statusLabel = new QLabel("System Owner • Online");
    statusLabel->setStyleSheet("color: #98C379; font-size: 14px; font-weight: bold;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(statusLabel);
    titleLayout->addStretch();

    headerLayout->addWidget(avatarPlaceholder);
    headerLayout->addSpacing(30);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // 2. Account Details Section
    QLabel* sectionTitle = new QLabel("ACCOUNT INFORMATION");
    sectionTitle->setStyleSheet("color: #56B6C2; font-weight: bold; letter-spacing: 2px;");
    mainLayout->addWidget(sectionTitle);

    mainLayout->addWidget(createInfoRow("User ID", "HUB-001-ADMIN"));
    mainLayout->addWidget(createInfoRow("Email", "admin@smarthome.pro"));
    mainLayout->addWidget(createInfoRow("Access Level", "Root / Level 4"));
    mainLayout->addWidget(createInfoRow("Last Login", QDateTime::currentDateTime().toString("MMM dd, yyyy - hh:mm")));

    mainLayout->addStretch(); // Push everything to the top
}

QWidget* ProfilePage::createInfoRow(QString label, QString value) {
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 10, 0, 10);

    QLabel* lblLabel = new QLabel(label);
    lblLabel->setStyleSheet("color: #ABB2BF; font-weight: bold; min-width: 150px;");

    QLabel* lblValue = new QLabel(value);
    lblValue->setStyleSheet("color: white;");

    layout->addWidget(lblLabel);
    layout->addWidget(lblValue);
    layout->addStretch();

    // Add a subtle bottom border to the row
    row->setStyleSheet("border-bottom: 1px solid #2C313C;");

    return row;
}