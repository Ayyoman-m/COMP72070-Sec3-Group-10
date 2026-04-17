#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVBoxLayout>
#include <QPainter>

/**
 * @class ClickableIcon
 * @brief Interactive device icon in the floor plan.
 *
 * Displays an icon that emits a signal when clicked.
 */
class ClickableIcon : public QGraphicsObject {
    Q_OBJECT
public:

    /**
     * @brief Constructs a clickable device icon.
     * @param pix Icon image
     * @param id Device identifier
     */
    ClickableIcon(const QPixmap& pix, QString id) : pixmap(pix), deviceId(id) {
        setCursor(Qt::PointingHandCursor);
    }

    QRectF boundingRect() const override { return QRectF(0, 0, 24, 24); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->drawPixmap(0, 0, 24, 24, pixmap);
    }

signals:

    /**
     * @brief Emitted when the icon is clicked.
     * @param id Device identifier
     */
    void clicked(QString id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override { emit clicked(deviceId); }

private:
    QPixmap pixmap;
    QString deviceId;
};

/**
 * @class ClickableRoomLabel
 * @brief Interactive room label in the floor plan.
 *
 * Displays room names and emits a signal when selected.
 */
class ClickableRoomLabel : public QGraphicsObject {
    Q_OBJECT
public:

    /**
    * @brief Constructs a clickable room label.
    * @param name Room name
    */
    ClickableRoomLabel(const QString& name) : roomName(name) {
        setCursor(Qt::PointingHandCursor);
    }

    QRectF boundingRect() const override { return QRectF(0, 0, 150, 30); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setPen(QColor("#61AFEF"));
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(boundingRect(), Qt::AlignLeft | Qt::AlignVCenter, roomName);
    }

signals:

    /**
     * @brief Emitted when a room label is clicked.
     * @param name Room name
     */
    void clicked(QString name);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override { emit clicked(roomName); }

private:
    QString roomName;
};

/**
 * @class MapPage
 * @brief Floor plan view for selecting rooms and devices.
 *
 * Displays an interactive map where users can click rooms
 * or devices to navigate or control them.
 */
class MapPage : public QWidget {
    Q_OBJECT
public:

    /**
     * @brief Constructs the MapPage.
     * @param parent Parent widget
     */
    explicit MapPage(QWidget* parent = nullptr);
signals:

    /**
     * @brief Emitted when a room is selected.
     * @param roomName Selected room name
     */
    void roomRequested(QString roomName);

    /**
     * @brief Emitted when a device is selected.
     * @param deviceId Selected device ID
     */
    void deviceRequested(QString deviceId);

private:

    /**
    * @brief Initializes UI components and scene.
    */
    void setupUi();

    /**
     * @brief Adds a room region to the map.
     */

    void addRoom(int x, int y, int w, int h, QString name);

    /**
     * @brief Adds a device icon to the map.
     */
    void addDevice(int x, int y, QString iconPath, QString id);

    QGraphicsView* view;
    QGraphicsScene* scene;
};