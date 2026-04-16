#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // try {
    //     ParsedData data = parseFile("/home/student/parcer/Transport-noise/st1_ac.00");

    //     // Теперь данные доступны в data.header и data.samples

    //     // Пример вывода (для проверки)
    //     qDebug() << "Частота:" << data.header.sampl_rate;

    //     // Для построения графика передайте QVector<double> или std::vector<double> в ваш виджет

    // } catch (const std::exception &e) {
    //     qDebug() << "Ошибка парсинга:" << e.what();
    // }

    return a.exec();
}
