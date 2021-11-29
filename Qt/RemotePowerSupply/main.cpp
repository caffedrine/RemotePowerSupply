#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_MESSAGE_PATTERN", QByteArray("[%{time yyyy-MM-dd h:mm:ss.zzz}] %{file}:%{line} - %{message}"));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
