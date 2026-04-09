#include <QApplication>
#include "SmartHomeServer.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    SmartHomeServer serverWindow;
    serverWindow.show();

    return a.exec();
}