#include "MapPage.h"
#include <QPen>
#include <QBrush>
#include <QIcon>

MapPage::MapPage(QWidget* parent) : QWidget(parent) {
    setupUi();
}

void MapPage::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(QBrush(QColor("#0B0E14")));

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("border: none; background: #0B0E14;");
    layout->addWidget(view);

    // --- DRAWING THE HOUSE PLAN ---
    addRoom(0, 0, 300, 250, "Living Room");
    addRoom(300, 0, 300, 250, "Kitchen");
    addRoom(0, 250, 200, 200, "Master Bedroom");
    addRoom(200, 250, 200, 200, "Guest Room");
    addRoom(400, 250, 200, 100, "Bathroom");
    addRoom(400, 350, 200, 100, "Garage");
    addRoom(0, 450, 600, 150, "Backyard");

    // --- ADDING DEVICE ICONS ---
    addDevice(50, 50, ":/icon_light", "LR_LIGHT_1");
    addDevice(250, 20, ":/icon_ac", "LR_AC");
    addDevice(350, 50, ":/icon_light", "KITCHEN_LIGHT");
    addDevice(500, 270, ":/icon_light", "BATH_LIGHT");
    addDevice(50, 300, ":/icon_fan", "MBR_FAN");
}

void MapPage::addRoom(int x, int y, int w, int h, QString name) {
    // 1. Draw the actual room rectangle (The walls/floor)
    QPen wallPen(QColor("#2C313C"), 2);
    scene->addRect(x, y, w, h, wallPen, QBrush(QColor("#161B22")));

    // 2. Add the clickable label
    ClickableRoomLabel* label = new ClickableRoomLabel(name);
    label->setPos(x + 15, y + 15); // Offset slightly from the corner

    // CHANGED: Use QObject::connect for the room label too
    QObject::connect(label, &ClickableRoomLabel::clicked, this, &MapPage::roomRequested);

    scene->addItem(label);
}

void MapPage::addDevice(int x, int y, QString iconPath, QString id) {
    QPixmap pix(iconPath);
    if (pix.isNull()) return;

    ClickableIcon* icon = new ClickableIcon(pix, id);
    icon->setPos(x, y);

    QObject::connect(icon, &ClickableIcon::clicked, this, &MapPage::deviceRequested);
    scene->addItem(icon);
}
