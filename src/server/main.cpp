#include <QApplication>
#include "SmartHomeServer.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // Only launch the Server window here
    SmartHomeServer serverWindow;
    serverWindow.setWindowTitle("Integrated Smart Home Server");
    serverWindow.resize(400, 300);
    serverWindow.show();

    return a.exec();
}