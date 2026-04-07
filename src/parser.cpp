#include "parser.h"

void parseFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла." << std::endl;
        return;
    }

    // Читаем заголовочную структуру
    PCHeader header{};
    file.read(reinterpret_cast<char *>(&header), sizeof(PCHeader));

    // Проверяем валидный формат файла
    if (!(header.id[0] == 'P' && header.id[1] == 'C')) {
        std::cerr << "Файл имеет неверный формат." << std::endl;
        return;
    }

    // Выводим важную информацию из заголовка
    std::cout << "Идентификатор формата: " << header.id.data() << '\n';
    std::cout << "Широта: " << header.lat << ", Долгота: " << header.lon << '\n';
    std::cout << "Коэффициент пересчета: " << header.scale << '\n';
    std::cout << "Время начала записи:\n";
    std::cout << "Год: " << static_cast<int>(header.year)
              << ", Месяц: " << static_cast<int>(header.month)
              << ", День: " << static_cast<int>(header.day) << '\n';
    std::cout << "Часы: " << static_cast<int>(header.hour)
              << ":Минуты: " << static_cast<int>(header.minute)
              << ":Секунды: " << static_cast<int>(header.second)
              << ".Микросекунды: " << header.microsec << '\n';
    std::cout << "Частота дискретизации: " << header.sampl_rate << " Гц\n";
    std::cout << "Количество отсчётов: " << header.num_samples << '\n';
    std::cout << "Тип отсчета: ";
    switch (static_cast<SampleTypes>(header.sample_type)) {
        case SampleTypes::SHORT: std::cout << "short"; break;
        case SampleTypes::INT: std::cout << "int"; break;
        case SampleTypes::FLOAT: std::cout << "float"; break;
        case SampleTypes::DOUBLE: std::cout << "double"; break;
        default: std::cout << "unknown type"; break;
    }
    std::cout << "\nНомер трассы: " << static_cast<int>(header.trace_num) << '\n';

    // Размер блока данных зависит от типа отсчета
    size_t bytesPerSample = [&]() {
        switch (static_cast<SampleTypes>(header.sample_type)) {
            case SampleTypes::SHORT: return sizeof(short); break;
            case SampleTypes::INT: return sizeof(int); break;
            case SampleTypes::FLOAT: return sizeof(float); break;
            case SampleTypes::DOUBLE: return sizeof(double); break;
            default: return sizeof(int);
            // default: throw std::runtime_error("Unknown data type");
        }
    }();

    // Выделяем обычную область памяти для временного хранения значений
    void *buffer = malloc(bytesPerSample);

    try {
        // Проход по каждому отсчёту
        for (size_t i = 0; i < header.num_samples; ++i) {
            file.read(static_cast<char *>(buffer), bytesPerSample);
            
            switch (static_cast<SampleTypes>(header.sample_type)) {
                case SampleTypes::SHORT: std::cout << *static_cast<short *>(buffer); break;
                case SampleTypes::INT: std::cout << *static_cast<int *>(buffer); break;
                case SampleTypes::FLOAT: std::cout << *static_cast<float *>(buffer); break;
                case SampleTypes::DOUBLE: std::cout << *static_cast<double *>(buffer); break;
                default: std::cout << *static_cast<int *>(buffer);
            }
            std::cout << '\n';
        }
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    // Освобождаем память вручную
    free(buffer);
}

int main() {
    const std::string filename = "../st1_ac.00";
    parseFile(filename);
    return 0;
}