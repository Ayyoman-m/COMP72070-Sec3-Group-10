#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QStackedWidget>
#include <QPushButton>

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

signals:
    void roomClicked(QString roomName);

private slots:
    void switchToGrid();
    void switchToList();

private:
    void setupUi();
    QWidget* createTopBar();
    QWidget* createGridView();
    QWidget* createListView();

    // Helpers
    QWidget* createRoomCard(QString name, QString status);
    QWidget* createRoomRow(QString name, QString status);

    // Navigation and View Management
    QStackedWidget* viewStack;
    QPushButton* btnGrid;
    QPushButton* btnList;

    // Status Labels (Requirement #6)
    QLabel* globalTempLabel;
    QLabel* activeLightsLabel;
};