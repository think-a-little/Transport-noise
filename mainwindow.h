#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "libs/qcustomplot/qcustomplot.h"
#include "../parcer/Transport-noise/src/parser.h"
#include <QFileSystemWatcher>
#include <QVector>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


// Структура для хранения одной точки данных (уже с абсолютным временем)
struct SignalRecord {
    QDateTime timestamp;
    double value;
};

// Структура, которую возвращает парсер, дополненная метаданными файла
struct ParsedFileData {
    QString fileName;
    QDateTime fileStartTime;
    double latitude;
    double longitude;
    std::vector<SignalRecord> records; // Используем вектор структур для удобства БД
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слот, реагирующий на появление нового файла
    void onNewFileDetected(const QString &path);

private:
    Ui::MainWindow *ui;
    QCustomPlot *customPlot; // Объявляем указатель на график

    // Данные для графика (накапливаются)
    QVector<double> m_allDataX; // UNIX-время для оси X
    QVector<double> m_allDataY; // Значения для оси Y

    // Объект для мониторинга папки
    QFileSystemWatcher *m_watcher;

    // --- Вспомогательные методы ---

    // Настройка внешнего вида графика и осей
    void setupPlot();

    // Обновление графика новыми данными
    void updatePlotWithNewData(const std::vector<SignalRecord> &newRecords);

    // Сохранение данных в PostgreSQL
    void saveDataToDatabase(const ParsedFileData &fileData);

    // Парсинг файла с преобразованием локального времени в абсолютное UTC/QDateTime
    ParsedFileData parseAndConvertFile(const QString &filename);
};
#endif // MAINWINDOW_H
