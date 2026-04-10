#include "mainwindow.h"
#include "../parcer/Transport-noise/src/parser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    const std::string filename = "/home/student/parcer/Transport-noise/st1_ac.00";

    try {
        parseFile(filename);
        return 0; // Успешное завершение
    } catch (const std::exception &ex) {
        // Выводим сообщение об ошибке в стандартный поток ошибок (stderr)
        std::cerr << "[ОШИБКА] " << ex.what() << '\n';
        return 1; // Код ошибки при завершении программы
    }

    return a.exec();
}
