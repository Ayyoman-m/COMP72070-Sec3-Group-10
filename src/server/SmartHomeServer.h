#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class SmartHomeServer : public QMainWindow {
    Q_OBJECT

public:
    SmartHomeServer(QWidget* parent = nullptr);
    ~SmartHomeServer() = default;

    // Public function so your future socket code can easily print here
    void logMessage(const QString& msg);

private slots:
    void toggleServer();

private:
    void setupUi();

    QPushButton* startStopBtn;
    QTextEdit* logConsole;
    QLabel* statusLabel;

    bool isRunning;
};