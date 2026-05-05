#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout> // Для удобного размещения виджетов
#include <QDebug>
#include <stdexcept> // Для обработки ошибок парсинга
#include <QDir>
#include <QDateTime>
#include <algorithm> // для std::copy

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    customPlot = new QCustomPlot(this);
    setCentralWidget(customPlot);

    setupPlot();

    // --- Инициализация мониторинга папки ---
    m_watcher = new QFileSystemWatcher(this);

    // Путь к папке для мониторинга. Создадим её в домашней директории.
    QString watchDirPath = QDir::homePath() + "/parcer/Transport-noise/src/Noise";
    QDir().mkpath(watchDirPath); // Создаем папку, если её нет

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::onNewFileDetected);

    m_watcher->addPath(watchDirPath);

    qDebug() << "Мониторинг запущен. Ожидаю файлы в папке:" << watchDirPath;
}

void MainWindow::setupPlot()
{
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));

    // 1. Создаем объект-тикер для даты и времени
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);

    // 2. Задаем формат отображения (часы:минуты:секунды.миллисекунды)
    dateTicker->setDateTimeFormat("hh:mm:ss.zzz");

    // 3. --- ВАЖНО ---
    // Говорим тикеру, что данные, которые он будет получать,
    // являются абсолютным временем в формате UTC (секунды от 1970 года).
    dateTicker->setDateTimeSpec(Qt::UTC);

    // 4. Устанавливаем этот тикер на ось X.
    customPlot->xAxis->setTicker(dateTicker);

    customPlot->xAxis->setLabel("Время (UTC)");
    customPlot->yAxis->setLabel("Амплитуда");

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::onNewFileDetected(const QString &path)
{
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);

    if (files.isEmpty()) {
        return;
    }

    // Берем последний добавленный файл (простой вариант)
    QString filename = dir.absoluteFilePath(files.last());

    // Проверяем, не обрабатываем ли мы файл повторно.
    // Простая проверка: если файл уже есть в нашей БД по имени, пропускаем.
    // (В продакшене лучше использовать более надежную систему).

    qDebug() << "Обнаружен новый файл:" << filename;

    try {
        ParsedFileData fileData = parseAndConvertFile(filename);

        if (!fileData.records.empty()) {
            saveDataToDatabase(fileData);
            updatePlotWithNewData(fileData.records);
        } else {
            qDebug() << "Файл пустой или не содержит данных.";
        }

        qDebug() << "Файл" << filename << "обработан успешно.";

        // Опционально: удалить файл после обработки или переместить его в архив
        // QFile::rename(filename, "/path/to/archive/" + QFileInfo(filename).fileName());

    } catch (const std::exception &e) {
        qDebug() << "Ошибка обработки файла" << filename << ":" << e.what();
        qDebug() << "Возможно, файл поврежден или имеет неверный формат.";
        // Файл можно переместить в папку "errors" для ручной проверки:
        // QDir().mkpath(QFileInfo(filename).absolutePath() + "/errors");
        // QFile::rename(filename, QFileInfo(filename).absolutePath() + "/errors/" + QFileInfo(filename).fileName());
    }
}

void MainWindow::updatePlotWithNewData(const std::vector<SignalRecord> &newRecords)
{
    int oldSize = m_allDataX.size();
    m_allDataX.reserve(oldSize + newRecords.size());
    m_allDataY.reserve(oldSize + newRecords.size());

    for (const auto &rec : newRecords) {
        // Получаем абсолютное время в секундах от начала эпохи (UNIX time)
        double absTimeSec = rec.timestamp.toMSecsSinceEpoch() / 1000.0;

        // Просто добавляем абсолютное время в секундах!
        // Благодаря setDateTimeSpec(Qt::UTC) в setupPlot,
        // график поймет, что это время и отобразит его правильно.
        m_allDataX.append(absTimeSec);
        m_allDataY.append(rec.value);
    }

    customPlot->graph(0)->setData(m_allDataX, m_allDataY);

    if (!m_allDataX.isEmpty()) {
        // Масштабируем ось X, чтобы показать последние 5 минут данных.
        // Так как время в секундах, 5 минут = 300 секунд.
        double lastTime = m_allDataX.last();
        customPlot->xAxis->setRange(lastTime - 300, lastTime);

        // Масштаб по Y остается прежним
        double minY = *std::min_element(m_allDataY.constBegin(), m_allDataY.constEnd());
        double maxY = *std::max_element(m_allDataY.constBegin(), m_allDataY.constEnd());
        double padding = (maxY - minY) * 0.05;
        customPlot->yAxis->setRange(minY - padding, maxY + padding);

        customPlot->replot();

        qDebug() << "График обновлен. Всего точек:" << m_allDataX.size();
    }
}

void MainWindow::saveDataToDatabase(const ParsedFileData &fileData)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    // --- ВНИМАНИЕ: ЗАМЕНИТЕ ДАННЫЕ НА СВОИ! ---
    db.setHostName("127.0.0.1");      // Адрес сервера БД (localhost)
    db.setDatabaseName("noisedb");// Имя вашей базы данных (например, 'seismic')
    db.setUserName("postgres");   // Ваш логин в PostgreSQL
    db.setPassword("32676");   // Ваш пароль

    if (!db.open()) {
        qDebug() << "Ошибка: не удалось подключиться к базе данных.";
        qDebug() << "Причина:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);

    query.prepare("INSERT INTO signal_data "
                  "(file_name, record_start_time, latitude, longitude, record_time, value) "
                  "VALUES (:file_name, :record_start_time, :latitude, :longitude, :record_time, :value)");

    // trace_number опущен для простоты примера

    db.transaction(); // Начинаем транзакцию для скорости

    bool success = true;

    for (const auto &rec : fileData.records) {
        query.bindValue(":file_name", fileData.fileName);
        query.bindValue(":record_start_time", fileData.fileStartTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));
        query.bindValue(":latitude", fileData.latitude);
        query.bindValue(":longitude", fileData.longitude);

        query.bindValue(":record_time", rec.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"));
        query.bindValue(":value", rec.value);

        if (!query.exec()) {
            qDebug() << "Ошибка вставки данных в БД:";
            qDebug() << "  Текст ошибки:" << query.lastError().text();
            success = false;
            break;
        }
    }

    if (success) {
        db.commit();   // Подтверждаем транзакцию
        qDebug() << "Успешно вставлено" << fileData.records.size() << "записей из файла" << fileData.fileName;
    } else {
        db.rollback(); // Откатываем изменения при ошибке
        qDebug() << "Транзакция отменена из-за ошибки.";
    }

    db.close(); // Закрываем соединение с БД после работы
}

// --- ЭТОТ МЕТОД НУЖНО ДОБАВИТЬ В ВАШ ПАРСЕР (parser.cpp) ---
// Он преобразует локальное время из заголовка в абсолютное время для каждой точки.
// Предполагается, что у вас есть структура PCHeader и функция parseFile.
// Вот как она может выглядеть:
ParsedFileData MainWindow::parseAndConvertFile(const QString &filename)
{
    ParsedFileData result;
    result.fileName = filename;

    // Здесь должен быть вызов вашего парсера или чтение бинарного файла напрямую.
    // Для примера предположим, что у вас есть функция parseFile,
    // которая возвращает структуру с заголовком и сырыми данными.

    // Парсим файл в промежуточную структуру (ваш старый парсер)
    ParsedData rawData = parseFile(filename.toStdString());

    result.fileStartTime = QDateTime(
        QDate(rawData.header.year, rawData.header.month, rawData.header.day),
        QTime(rawData.header.hour, rawData.header.minute, rawData.header.second),
        Qt::UTC // Важно! Указываем часовой пояс UTC или нужный вам.
        );

    result.latitude = rawData.header.lat;
    result.longitude = rawData.header.lon;

    double dt = 1.0 / static_cast<double>(rawData.header.sampl_rate);

    for (size_t i = 0; i < rawData.samples.size(); ++i) {
        SignalRecord rec;
        rec.timestamp = result.fileStartTime.addMSecs(static_cast<qint64>(i * dt * 1000));
        rec.value = rawData.samples[i];
        result.records.push_back(rec);
    }

    return result;
}

MainWindow::~MainWindow()
{
    delete ui;
}
