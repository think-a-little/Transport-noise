#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "libs/qcustomplot/qcustomplot.h"
#include "../parcer/Transport-noise/src/parser.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot; // Объявляем указатель на график

    void setupPlot(); // Функция для настройки интерфейса с графиком
    void plotData(const ParsedData &data); // Функция для отрисовки данных
};
#endif // MAINWINDOW_H
