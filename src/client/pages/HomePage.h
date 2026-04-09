#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

// Forward declaration tells the compiler these classes exist 
// without needing to include their massive headers here.
class QComboBox;
class QGridLayout;

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

signals:
    void roomClicked(const QString& roomName);
    void modeChangeRequested(int modeIndex); // REQ-SVR-040

private slots:
    void switchToGrid();
    void switchToList();

private:
    void setupUi();
    QWidget* createTopBar();
    QWidget* createGridView();
    QWidget* createListView();
    QWidget* createRoomCard(QString name, QString status);
    QWidget* createRoomRow(QString name, QString status);

    // --- The "Missing" Members causing your errors ---
    QStackedWidget* viewStack;
    QLabel* globalTempLabel;
    QComboBox* modeSelector;   // Line 37 - Fixed!
    QPushButton* btnGrid;
    QPushButton* btnList;
};