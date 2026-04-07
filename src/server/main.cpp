#include "SmartHomeServer.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    SmartHomeServer serverWindow;
    serverWindow.show();

    return a.exec();
}