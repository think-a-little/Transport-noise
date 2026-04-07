#include "mainwindow.h"
#include "../parcer/Transport-noise/src/parser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    const std::string filename = "/home/student/parcer/Transport-noise/st1_ac.00";
    parseFile(filename);
    return a.exec();
}
