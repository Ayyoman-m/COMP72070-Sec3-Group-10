#pragma once

#include <QtWidgets/QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

class SmartHomeClient : public QMainWindow {
    Q_OBJECT

public:
    SmartHomeClient(QWidget* parent = nullptr);
    ~SmartHomeClient() = default;

private slots:
    void attemptLogin();
    void sendCommand();
    void onCommandTimeout();
    void simulateServerResponse(); // Placeholder until POSIX sockets are wired

private:
    void setupUi();
    void setupLoginScreen();
    void setupDashboard();

    // UI Navigation
    QStackedWidget* centralStack;
    QWidget* loginWidget;
    QWidget* dashboardWidget;

    // Login Elements
    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* loginStatusLabel;

    // Dashboard Elements
    QLabel* modeLabel;
    QLabel* feedbackLabel;
    QPushButton* sendCommandBtn;

    // Interactive Floorplan
    QGraphicsView* floorplanView;
    QGraphicsScene* floorplanScene;
    QGraphicsRectItem* testAppliance; // Represents a clickable device

    // Networking / Logic
    QTimer* commandTimer;
    bool isCommandPending;
};