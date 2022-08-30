#include "mainwindow.h"
#include <QApplication>
#include "Logger.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Show debug info when app is executed from console
    #ifdef _WIN32
        /// https://stackoverflow.com/questions/3360548/console-output-in-a-qt-gui-app
        if (AttachConsole(ATTACH_PARENT_PROCESS))
        {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }
    #endif

    setup_logger();
    logger->info("App started...");

    MainWindow w;
    w.show();
    return a.exec();
}
