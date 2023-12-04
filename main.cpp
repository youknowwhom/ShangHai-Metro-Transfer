#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/font/corbel.ttf");
    QFontDatabase::addApplicationFont(":/font/corbell.ttf");
    QFontDatabase::addApplicationFont(":/font/corbelb.ttf");
    QFontDatabase::addApplicationFont(":/font/corbeli.ttf");
    QFontDatabase::addApplicationFont(":/font/corbell.ttf");
    QFontDatabase::addApplicationFont(":/font/corbelli.ttf");

    QFontDatabase::addApplicationFont(":/font/MiSans-Demibold.ttf");
    QFontDatabase::addApplicationFont(":/font/MiSans-Light.ttf");
    QFontDatabase::addApplicationFont(":/font/MiSans-Regular.ttf");

    MainWindow w;
    w.show();

    return a.exec();
}
