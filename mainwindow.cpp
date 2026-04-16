#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout> // Для удобного размещения виджетов
#include <QDebug>
#include <stdexcept> // Для обработки ошибок парсинга

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupPlot(); // Настраиваем интерфейс с графиком

    // --- БЛОК ПАРСИНГА И ОТОБРАЖЕНИЯ ДАННЫХ ---
    try {
        // ВАЖНО: Укажите правильный путь к вашему файлу!
        QString filePath = "/home/student/parcer/Transport-noise/st1_ac.00";
        ParsedData data = parseFile(filePath.toStdString());

        // Вызываем функцию отрисовки, передав ей данные и виджет графика
        plotData(data);

        qDebug() << "График успешно построен.";

    } catch (const std::exception &e) {
        // Если файл не найден или возникла ошибка, выводим сообщение в отладчик
        qDebug() << "Ошибка при загрузке или парсинге файла:" << e.what();

        // Можно также вывести сообщение пользователю через QLabel на форме
        // ui->labelStatus->setText("Ошибка загрузки данных");
    }
}

void MainWindow::setupPlot()
{
    // Создаем объект для рисования графика
    customPlot = new QCustomPlot(this);

    // Создаем Layout, чтобы график занял все доступное место в окне
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    layout->addWidget(customPlot); // Добавляем график в layout
    this->setCentralWidget(centralWidget); // Устанавливаем центральный виджет окна

    // Настраиваем взаимодействие: зум колесом и перетаскивание
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::plotData(const ParsedData &data)
{
    if (data.samples.empty()) {
        qDebug() << "Нет данных для построения графика.";
        return;
    }

    // 1. Добавляем график на полотно (индекс 0)
    customPlot->addGraph();

    // 2. Создаем вектора для осей X и Y.
    // Размер векторов равен количеству отсчетов.
    QVector<double> x(data.header.num_samples), y(data.header.num_samples);

    // 3. Вычисляем шаг по времени (dt)
    double dt = 1.0 / static_cast<double>(data.header.sampl_rate);

    // 4. Заполняем вектора данными из структуры ParsedData
    for (int i = 0; i < data.header.num_samples; ++i) {
        x[i] = i * dt; // Координата X: время (сек)
        y[i] = data.samples[i]; // Координата Y: значение сигнала (амплитуда)
    }

    // 5. Передаем данные на график с индексом 0
    customPlot->graph(0)->setData(x, y);

    // 6. Настраиваем внешний вид линии графика (цвет, толщина)
    customPlot->graph(0)->setPen(QPen(Qt::blue));

    // 7. Настраиваем оси и их подписи
    customPlot->xAxis->setLabel("Время (сек)");
    customPlot->yAxis->setLabel("Амплитуда");

    // Автоматически подгоняем масштаб осей под данные с небольшим запасом (padding)
    customPlot->rescaleAxes(true);

    // Принудительная перерисовка всего полотна графика с новыми данными и настройками.
    customPlot->replot();
}

MainWindow::~MainWindow()
{
    delete ui;
}
