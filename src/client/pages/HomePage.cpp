#include "HomePage.h"
#include "../StyleManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QComboBox>

/**
 * @brief Constructs the HomePage dashboard.
 *
 * Initializes the main layout and sets up UI components.
 */
HomePage::HomePage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

/**
 * @brief Sets up the main UI layout for the dashboard.
 *
 * Creates the top bar and initializes grid/list view switching.
 */
void HomePage::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

     /**
     * @brief Creates the top status bar for the dashboard.
     *
     * Includes system mode selector, temperature display,
     * and view toggle buttons.
     *
     * Implements REQ-SVR-030 (mode selection).
     */
    mainLayout->addWidget(createTopBar());

    // 2. Setup the view stack for Grid/List toggling
    viewStack = new QStackedWidget();
    viewStack->addWidget(createGridView());
    viewStack->addWidget(createListView());

    mainLayout->addWidget(viewStack);

    switchToGrid(); // Default to Grid view
}

QWidget* HomePage::createTopBar() {
    QWidget* bar = new QWidget();
    bar->setObjectName("StatusBar");
    bar->setFixedHeight(70);
    bar->setStyleSheet(StyleManager::getStatusBarStyle());

    QHBoxLayout* layout = new QHBoxLayout(bar);

    // --- INITIALIZATION (Critical to prevent crashes) ---

    // Global Status
    globalTempLabel = new QLabel("Home Avg: 21.8°C");
    globalTempLabel->setStyleSheet("color: #ABB2BF; font-weight: bold;");

    // Mode Selector (Fulfills REQ-SVR-030)
    modeSelector = new QComboBox();
    modeSelector->addItems({ "LOCKED", "HOME", "AWAY", "MAINTENANCE" });
    modeSelector->setFixedWidth(130);
    modeSelector->setStyleSheet(
        "QComboBox { background: #2C313C; color: #61AFEF; border: 1px solid #61AFEF; border-radius: 4px; padding: 5px; font-weight: bold; }"
        "QComboBox QAbstractItemView { background-color: #1E222A; color: #ABB2BF; selection-background-color: #61AFEF; }"
    );

    connect(modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        emit modeChangeRequested(index);
        });

    // Toggle Buttons
    btnGrid = new QPushButton("GRID");
    btnList = new QPushButton("LIST");
    btnGrid->setFixedSize(60, 30);
    btnList->setFixedSize(60, 30);

    connect(btnGrid, &QPushButton::clicked, this, &HomePage::switchToGrid);
    connect(btnList, &QPushButton::clicked, this, &HomePage::switchToList);

    // --- LAYOUT ARRANGEMENT ---
    layout->addWidget(new QLabel("<h3 style='color:#61AFEF;'>DASHBOARD</h3>"));
    layout->addStretch();

    layout->addWidget(new QLabel("SYSTEM MODE:"));
    layout->addWidget(modeSelector);
    layout->addSpacing(20);

    layout->addWidget(globalTempLabel);
    layout->addSpacing(40);

    layout->addWidget(btnGrid);
    layout->addWidget(btnList);

    return bar;
}

/**
 * @brief Switches the view to grid layout.
 */
void HomePage::switchToGrid() {
    viewStack->setCurrentIndex(0);
    btnGrid->setStyleSheet(StyleManager::getToggleButtonStyle(true));
    btnList->setStyleSheet(StyleManager::getToggleButtonStyle(false));
}

/**
 * @brief Switches the view to list layout.
 */
void HomePage::switchToList() {
    viewStack->setCurrentIndex(1);
    btnGrid->setStyleSheet(StyleManager::getToggleButtonStyle(false));
    btnList->setStyleSheet(StyleManager::getToggleButtonStyle(true));
}

/**
 * @brief Creates the grid view layout for rooms.
 *
 * Displays rooms as interactive cards.
 */
QWidget* HomePage::createGridView() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget* container = new QWidget();
    QGridLayout* grid = new QGridLayout(container);
    grid->setSpacing(20);

    QStringList rooms = { "Living Room", "Kitchen", "Master Bedroom", "Guest Room", "Bathroom", "Garage", "Backyard" };
    int row = 0, col = 0;
    for (const QString& name : rooms) {
        grid->addWidget(createRoomCard(name, "Active"), row, col);
        col++;
        if (col > 2) { col = 0; row++; }
    }

    scroll->setWidget(container);
    return scroll;
}

/**
 * @brief Creates the list view layout for rooms.
 *
 * Displays rooms in a vertical list format.
 */
QWidget* HomePage::createListView() {
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    QWidget* container = new QWidget();
    QVBoxLayout* list = new QVBoxLayout(container);
    list->setSpacing(10);

    QStringList rooms = { "Living Room", "Kitchen", "Master Bedroom", "Guest Room", "Bathroom", "Garage", "Backyard" };
    for (const QString& name : rooms) {
        list->addWidget(createRoomRow(name, "Online"));
    }
    list->addStretch();

    scroll->setWidget(container);
    return scroll;
}

/**
 * @brief Creates a clickable room card.
 *
 * @param name Room name
 * @param status Current room status
 */
QWidget* HomePage::createRoomCard(QString name, QString status) {
    QFrame* card = new QFrame();
    card->setObjectName("RoomCard");
    card->setFixedSize(250, 150);
    card->setStyleSheet(StyleManager::getRoomCardStyle());

    QVBoxLayout* l = new QVBoxLayout(card);

    QLabel* nameLbl = new QLabel("<b>" + name + "</b>");
    nameLbl->setStyleSheet("font-size: 16px; color: white;");

    QLabel* statLbl = new QLabel(status);
    statLbl->setStyleSheet("color: #98C379; font-size: 12px;");

    l->addWidget(nameLbl);
    l->addWidget(statLbl);
    l->addStretch();

    QPushButton* btn = new QPushButton(card);
    btn->setFixedSize(250, 150);
    btn->setStyleSheet("background:transparent; border:none;");
    btn->setCursor(Qt::PointingHandCursor);
    connect(btn, &QPushButton::clicked, [this, name]() { emit roomClicked(name); });

    return card;
}

/**
 * @brief Creates a row entry for a room in list view.
 *
 * @param name Room name
 * @param status Current room status
 */
QWidget* HomePage::createRoomRow(QString name, QString status) {
    QWidget* row = new QWidget();
    row->setStyleSheet("background: #1E222A; border-radius: 8px; border: 1px solid #2C313C;");
    QHBoxLayout* l = new QHBoxLayout(row);
    l->setContentsMargins(15, 10, 15, 10);

    QLabel* nameLbl = new QLabel("<b>" + name + "</b>");
    nameLbl->setStyleSheet("color: white; font-size: 14px;");

    l->addWidget(nameLbl);
    l->addStretch();

    QLabel* statLbl = new QLabel(status);
    statLbl->setStyleSheet("color: #ABB2BF; margin-right: 20px;");
    l->addWidget(statLbl);

    QPushButton* btn = new QPushButton("VIEW");
    btn->setFixedSize(70, 30);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton { background: #161B22; color: #61AFEF; border: 1px solid #61AFEF; border-radius: 4px; font-weight: bold; font-size: 11px; }"
        "QPushButton:hover { background: #61AFEF; color: #12151A; }"
    );
    connect(btn, &QPushButton::clicked, [this, name]() { emit roomClicked(name); });
    l->addWidget(btn);

    return row;
}