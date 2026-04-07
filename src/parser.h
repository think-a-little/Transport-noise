#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <cstdint>

// Определим структуру заголовка файла
struct PCHeader {
    std::array<char, 2> id; // 'P', 'C'
    std::array<char, 4> reserved;
    float lat;
    float lon;
    double scale;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int microsec;
    int16_t sampl_rate;
    int32_t num_samples;
    int16_t sample_type;
    uint8_t trace_num;
    uint8_t reserved_byte;
};

// Классическое перечисление с типом
enum class SampleTypes : int16_t { SHORT = 0x0002, INT = 0x0004, FLOAT = 0x1004, DOUBLE = 0x1008 };

void parseFile(const std::string &filename);