/**
 * @file main.cpp
 * @brief Entry point for the Smart Home Server application.
 *
 * Initializes the Qt application and launches the server interface.
 */

#include <QApplication>
#include "SmartHomeServer.h"

 /**
  * @brief Main function for starting the server application.
  *
  * @param argc Argument count
  * @param argv Argument vector
  * @return Application exit status
  */
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    SmartHomeServer serverWindow;
    serverWindow.show();

    return a.exec();
}