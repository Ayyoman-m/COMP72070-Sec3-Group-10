#pragma once
#include <QString>

namespace StyleManager {

    // --- COLOR PALETTE ---
    const QString COLOR_BG_DEEP = "#0B0E14";
    const QString COLOR_BG_CARD = "#161B22";
    const QString COLOR_BG_SIDEBAR = "#12151A";
    const QString COLOR_ACCENT_BLUE = "#61AFEF";
    const QString COLOR_ACCENT_RED = "#E06C75";
    const QString COLOR_ACCENT_GREEN = "#98C379";
    const QString COLOR_ACCENT_CYAN = "#56B6C2";
    const QString COLOR_TEXT_MAIN = "#ABB2BF";
    const QString COLOR_BORDER = "#2C313C";

    // --- MAIN WINDOW ---
    inline QString getMainWindowStyle() {
        return QString("QMainWindow { background-color: %1; }").arg(COLOR_BG_DEEP);
    }

    // --- STATUS BAR ---
    inline QString getStatusBarStyle() {
        return QString(
            "QWidget#StatusBar { "
            "  background-color: #1E222A; "
            "  border-bottom: 2px solid %1; "
            "  border-radius: 5px; "
            "}"
            "QLabel { font-weight: bold; color: %2; font-size: 13px; }"
        ).arg(COLOR_BORDER, COLOR_TEXT_MAIN);
    }

    // --- SEGMENTED TOGGLE ---
    inline QString getToggleButtonStyle(bool active) {
        if (active) {
            return QString(
                "QPushButton { "
                "  background-color: %1; "
                "  color: #12151A; "
                "  border-radius: 4px; "
                "  font-weight: bold; "
                "}"
            ).arg(COLOR_ACCENT_BLUE);
        }
        else {
            return QString(
                "QPushButton { "
                "  background-color: #1E222A; "
                "  color: %1; "
                "  border: 1px solid %2; "
                "  border-radius: 4px; "
                "}"
            ).arg(COLOR_TEXT_MAIN, COLOR_BORDER);
        }
    }

    // --- ROOM CARDS (The one causing the error!) ---
    inline QString getRoomCardStyle() {
        return QString(
            "QFrame#RoomCard { "
            "  background-color: %1; "
            "  border: 1px solid %2; "
            "  border-radius: 12px; "
            "}"
            "QFrame#RoomCard:hover { "
            "  border: 1px solid %3; "
            "  background-color: #1c2128; "
            "}"
        ).arg(COLOR_BG_CARD, COLOR_BORDER, COLOR_ACCENT_BLUE);
    }

    // --- DEVICE CONTROL CARDS ---
    inline QString getDeviceControlStyle() {
        return QString(
            "QFrame#DeviceCard { "
            "  background-color: #161B22; "
            "  border: 1px solid %1; "
            "  border-radius: 10px; "
            "}"
            "QPushButton#ToggleOn { background-color: %2; color: #12151A; font-weight: bold; }"
            "QPushButton#ToggleOff { background-color: #3E4451; color: white; }"
        ).arg(COLOR_BORDER, COLOR_ACCENT_GREEN);
    }

    // --- SIDEBAR ---
    inline QString getSidebarStyle() {
        return QString(
            "QWidget#Sidebar { background-color: %1; border-right: 1px solid %2; }"
            "QPushButton { background-color: transparent; border: none; color: %3; text-align: left; padding: 12px; }"
            "QPushButton:hover { background-color: #1E222A; color: %4; }"
        ).arg(COLOR_BG_SIDEBAR, COLOR_BORDER, COLOR_TEXT_MAIN, COLOR_ACCENT_BLUE);
    }

    // --- INPUT FIELDS ---
    inline QString getLoginInputStyle() {
        return QString(
            "QLineEdit { "
            "  background-color: #1E222A; "
            "  border: 1px solid %1; "
            "  color: white; "
            "  padding: 10px; "
            "  border-radius: 5px; "
            "}"
            "QLineEdit:focus { border: 1px solid %2; }"
        ).arg(COLOR_BORDER, COLOR_ACCENT_BLUE);
    }
}