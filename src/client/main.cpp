#include "SmartHomeClient.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    SmartHomeClient clientWindow;
    clientWindow.show();

    return a.exec();
}