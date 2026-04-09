#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QVBoxLayout>
#include <QPainter>

// --- CLEAN CLICKABLE ICON ---
class ClickableIcon : public QGraphicsObject {
    Q_OBJECT
public:
    ClickableIcon(const QPixmap& pix, QString id) : pixmap(pix), deviceId(id) {
        setCursor(Qt::PointingHandCursor);
    }

    // Must override these for QGraphicsObject
    QRectF boundingRect() const override { return QRectF(0, 0, 24, 24); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->drawPixmap(0, 0, 24, 24, pixmap);
    }

signals:
    void clicked(QString id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override { emit clicked(deviceId); }

private:
    QPixmap pixmap;
    QString deviceId;
};

// --- CLEAN CLICKABLE ROOM LABEL ---
class ClickableRoomLabel : public QGraphicsObject {
    Q_OBJECT
public:
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
    void clicked(QString name);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override { emit clicked(roomName); }

private:
    QString roomName;
};

class MapPage : public QWidget {
    Q_OBJECT
public:
    explicit MapPage(QWidget* parent = nullptr);
signals:
    void roomRequested(QString roomName);
    void deviceRequested(QString deviceId);
private:
    void setupUi();
    void addRoom(int x, int y, int w, int h, QString name);
    void addDevice(int x, int y, QString iconPath, QString id);
    QGraphicsView* view;
    QGraphicsScene* scene;
};