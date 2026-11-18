// Программа замеров времени (DESIGN.md, п. 3.3 «Замеры», п. 4.9).
//
// Запускает эксперимент с настройками по умолчанию, печатает таблицу хода
// эксперимента и сохраняет результаты в CSV (по умолчанию
// bench/results/results.csv). Графики строит bench/plot.py.
//
// Смысл имеют только замеры в сборке Release (пресет release).
//
// Параметры командной строки:
//   --quick        размеры 10³ и 10⁴, одно построение — для проверки
//   --out ФАЙЛ     куда сохранить CSV

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "Benchmark.h"
#include "Exceptions.h"
#include "TextUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef LAB2_RESULTS_DIR
#define LAB2_RESULTS_DIR "."
#endif

namespace
{

void SetUpConsoleEncoding()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

// Ширина столбцов таблицы в символах.
const std::size_t kSizeWidth = 9;
const std::size_t kOrderWidth = 16;
const std::size_t kMethodWidth = 16;
const std::size_t kNumberWidth = 16;
const int kDigits = 3;

void PrintHeader()
{
    std::cout << PadRight("размер", kSizeWidth) << PadRight("порядок", kOrderWidth)
              << PadRight("метод", kMethodWidth) << PadLeft("построение, мс", kNumberWidth)
              << PadLeft("по ключу, мкс", kNumberWidth)
              << PadLeft("диапазон, мкс", kNumberWidth) << "\n";
}

void PrintMeasurement(const Measurement& m)
{
    std::cout << PadRight(std::to_string(m.size), kSizeWidth)
              << PadRight(GetOrderName(m.order), kOrderWidth)
              << PadRight(GetMethodName(m.method), kMethodWidth);
    if (m.skipped)
    {
        std::cout << PadLeft("пропущен, O(n²)", kNumberWidth) << "\n";
        return;
    }
    std::cout << PadLeft(FormatFixed(m.buildMs, kDigits), kNumberWidth)
              << PadLeft(FormatFixed(m.findUs, kDigits), kNumberWidth)
              << PadLeft(FormatFixed(m.rangeUs, kDigits), kNumberWidth) << "\n";
}

// Разбирает параметры командной строки. Возвращает false при неизвестном
// параметре (сообщение уже выведено).
bool ParseArguments(int argc, char** argv, ExperimentConfig& config, std::string& outputPath)
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--quick") == 0)
        {
            const std::size_t thousand = 1000;
            config.sizes = ArraySequence<std::size_t>{thousand, 10 * thousand};
            config.repeats = 1;
        }
        else if (std::strcmp(argv[i], "--out") == 0 && i + 1 < argc)
        {
            ++i;
            outputPath = argv[i];
        }
        else
        {
            std::cerr << "Неизвестный параметр «" << argv[i]
                      << "». Допустимо: --quick, --out ФАЙЛ\n";
            return false;
        }
    }
    return true;
}

// Проводит эксперимент с выводом хода в консоль и сохраняет CSV.
void RunAndSave(const ExperimentConfig& config, const std::string& outputPath)
{
    std::string currentAttribute;
    const ArraySequence<Measurement> measurements =
        RunExperiment(config, [&currentAttribute](const Measurement& m)
                      {
                          if (m.attribute != currentAttribute)
                          {
                              currentAttribute = m.attribute;
                              std::cout << "\nАтрибут: " << currentAttribute << "\n";
                              PrintHeader();
                          }
                          PrintMeasurement(m);
                      });

    // u8path: путь в UTF-8 может содержать русские буквы (в Windows иначе не откроется).
    std::ofstream out(std::filesystem::u8path(outputPath));
    if (!out)
    {
        throw FileError("Не удалось создать файл «" + outputPath + "»");
    }
    WriteMeasurementsCsv(measurements, out);
    std::cout << "\nРезультаты сохранены в " << outputPath << "\n";
}

} // namespace

int main(int argc, char** argv)
{
    SetUpConsoleEncoding();

    ExperimentConfig config = MakeDefaultExperimentConfig();
    std::string outputPath = std::string(LAB2_RESULTS_DIR) + "/results.csv";
    if (!ParseArguments(argc, argv, config, outputPath))
    {
        return 1;
    }
    try
    {
        RunAndSave(config, outputPath);
    }
    catch (const Lab2Error& error)
    {
        std::cerr << "Ошибка: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
