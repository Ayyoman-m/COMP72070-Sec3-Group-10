#include "SmartHomeClient.h"
#include <QtWidgets/QApplication>
#include <winsock2.h>


int main(int argc, char* argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1; // Initialization failed
    }

    QApplication a(argc, argv);
    SmartHomeClient clientWindow;
    clientWindow.show();

    return a.exec();

    int result = a.exec();

    WSACleanup();
    return result;
}