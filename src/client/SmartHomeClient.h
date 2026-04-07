#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QGroupBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>

class SmartHomeClient : public QMainWindow { // Changed to QMainWindow to match .cpp
    Q_OBJECT

public:
    explicit SmartHomeClient(QWidget* parent = nullptr);
    ~SmartHomeClient();

private slots:
    // Functional slots called by the UI
    void attemptLogin();
    void sendCommand();
    void onCommandTimeout();
    void simulateServerResponse();

private:
    // Initialization methods
    void setupUi();
    void setupLoginScreen();
    void setupDashboard();

    // UI Layout Elements
    QStackedWidget* centralStack;
    QWidget* loginWidget;
    QWidget* dashboardWidget;

    // Login Screen Elements
    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* loginStatusLabel;

    // Dashboard Elements
    QGroupBox* statusGroup;
    QLabel* modeLabel;
    QGraphicsView* floorplanView;
    QGraphicsScene* floorplanScene;
    QGraphicsRectItem* testAppliance; // Changed to RectItem based on .cpp usage

    QPushButton* sendCommandBtn;
    QLabel* feedbackLabel;

    // State Variables
    QTimer* commandTimer;
    bool isCommandPending;
};