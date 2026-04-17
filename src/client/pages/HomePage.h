#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

// Forward declaration tells the compiler these classes exist 
// without needing to include their massive headers here.
class QComboBox;
class QGridLayout;

/**
 * @class HomePage
 * @brief Dashboard page displaying rooms and system status.
 *
 * Provides grid and list views of rooms and allows users to
 * select rooms and change system mode.
 */
class HomePage : public QWidget {
    Q_OBJECT

public:

    /**
     * @brief Constructs the HomePage.
     * @param parent Parent widget
     */
    explicit HomePage(QWidget* parent = nullptr);

signals:

    /**
     * @brief Emitted when a room is selected.
     * @param roomName Name of selected room
     */
    void roomClicked(const QString& roomName);

    /**
     * @brief Emitted when system mode is changed.
     *
     * Implements REQ-SVR-040 (state/mode change).
     *
     * @param modeIndex Selected mode index
     */
    void modeChangeRequested(int modeIndex); 

private slots:

    /**
     * @brief Switches view to grid layout.
     */
    void switchToGrid();

    /**
     * @brief Switches view to list layout.
     */
    void switchToList();

private:

    /**
   * @brief Initializes UI layout.
   */
    void setupUi();

    /**
    * @brief Creates the top status bar.
    */
    QWidget* createTopBar();

    /**
     * @brief Creates grid view for rooms.
     */
    QWidget* createGridView();

    /**
     * @brief Creates list view for rooms.
     */
    QWidget* createListView();

    /**
     * @brief Creates a room card widget.
     */
    QWidget* createRoomCard(QString name, QString status);

    /**
     * @brief Creates a row entry for a room.
     */
    QWidget* createRoomRow(QString name, QString status);

    // --- The "Missing" Members causing your errors ---
    QStackedWidget* viewStack;
    QLabel* globalTempLabel;
    QComboBox* modeSelector;   // Line 37 - Fixed!
    QPushButton* btnGrid;
    QPushButton* btnList;
};