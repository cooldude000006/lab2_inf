// Консольный интерфейс: поиск, сравнение методов, эксперимент.
// Меню, данные и индексы — в ConsoleUI.cpp.

#include "ConsoleUI.h"

#include <chrono>
#include <climits>
#include <filesystem>
#include <fstream>

#include "Exceptions.h"
#include "LinearSearch.h"
#include "TextUtils.h"

namespace
{

using Clock = std::chrono::steady_clock;

const std::size_t kMaxShownResults = 20;
const int kMaxExperimentSize = 2000000;
const int kMaxRepeats = 20;
const int kDigits = 3;

// Ширина столбцов таблицы замеров в символах.
const std::size_t kSizeWidth = 9;
const std::size_t kOrderWidth = 16;
const std::size_t kMethodWidth = 16;
const std::size_t kValueWidth = 10;

double MicrosecondsSince(Clock::time_point start)
{
    return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

std::string FormatTime(double microseconds)
{
    const double microsecondsPerMillisecond = 1000.0;
    if (microseconds < microsecondsPerMillisecond)
    {
        return FormatFixed(microseconds, 1) + " мкс";
    }
    return FormatFixed(microseconds / microsecondsPerMillisecond, kDigits) + " мс";
}

} // namespace

// --- Поиск ------------------------------------------------------------------

void ConsoleUI::SearchMenu()
{
    std::size_t number = 0;
    if (!ChooseIndex(number))
    {
        return;
    }
    const Index& index = database_.GetIndex(number);
    const int choice = io_.ReadMenuChoice(
        "Поиск: " + index.GetAttribute().GetName() + ", " + GetMethodName(index.GetMethod()),
        ArraySequence<std::string>{"По ключу", "По диапазону ключей"}, "Назад");
    if (choice == 1)
    {
        SearchByKey(index);
    }
    else if (choice == 2)
    {
        SearchByRange(index);
    }
}

void ConsoleUI::SearchByKey(const Index& index)
{
    const CompositeAttribute& attribute = index.GetAttribute();
    const CompositeKey key = ReadKey(attribute, "Ключ");

    Clock::time_point start = Clock::now();
    const Index::Positions found = index.Find(key);
    const double elapsedUs = MicrosecondsSince(start);

    start = Clock::now();
    const Index::Positions expected = LinearFind(database_.GetPeople(), attribute, key);
    const double linearUs = MicrosecondsSince(start);

    ReportSearch(index, found, elapsedUs, expected, linearUs);
}

void ConsoleUI::SearchByRange(const Index& index)
{
    const CompositeAttribute& attribute = index.GetAttribute();
    const CompositeKey low = ReadKey(attribute, "Нижняя граница (включается)");
    const CompositeKey high = ReadKey(attribute, "Верхняя граница (включается)");

    Clock::time_point start = Clock::now();
    const Index::Positions found = index.FindRange(low, high);
    const double elapsedUs = MicrosecondsSince(start);

    start = Clock::now();
    const Index::Positions expected = LinearFindRange(database_.GetPeople(), attribute, low, high);
    const double linearUs = MicrosecondsSince(start);

    ReportSearch(index, found, elapsedUs, expected, linearUs);
}

// Вывод результата и автоматическая проверка: результат индекса сравнивается
// с линейным поиском по тем же данным.
void ConsoleUI::ReportSearch(const Index& index, const Index::Positions& found,
                             double elapsedUs, const Index::Positions& expected, double linearUs)
{
    io_.Out() << "Найдено записей: " << found.GetLength() << " за " << FormatTime(elapsedUs)
              << " (" << GetMethodName(index.GetMethod()) << ").\n";
    PrintFound(found);

    if (index.GetMethod() == SearchMethod::Linear)
    {
        return;
    }
    io_.Out() << "Проверка линейным поиском (" << FormatTime(linearUs) << "): ";
    if (found == expected)
    {
        io_.Out() << "результаты совпадают.";
        if (elapsedUs > 0.0)
        {
            io_.Out() << " Индекс быстрее в " << FormatFixed(linearUs / elapsedUs, 1) << " раз.";
        }
        io_.Out() << "\n";
    }
    else
    {
        io_.Out() << "РЕЗУЛЬТАТЫ РАСХОДЯТСЯ: линейный поиск нашёл " << expected.GetLength()
                  << " записей.\n";
    }
}

void ConsoleUI::PrintFound(const Index::Positions& positions)
{
    if (positions.GetIsEmpty())
    {
        return;
    }
    PrintPeopleHeader();
    for (std::size_t i = 0; i < positions.GetLength() && i < kMaxShownResults; ++i)
    {
        PrintPerson(positions.Get(i));
    }
    if (positions.GetLength() > kMaxShownResults)
    {
        io_.Out() << "… и ещё " << positions.GetLength() - kMaxShownResults << "\n";
    }
}

// --- Сравнение методов на текущих данных ------------------------------------

void ConsoleUI::CompareMethods()
{
    if (database_.GetCount() == 0)
    {
        io_.Out() << "Записей нет. Сгенерируйте или загрузите данные.\n";
        return;
    }
    io_.Out() << "Все методы строят индекс по одному атрибуту на текущих данных и выполняют "
                 "одни и те же запросы.\n";
    const CompositeAttribute attribute = ReadCompositeAttribute();

    ExperimentConfig config = MakeDefaultExperimentConfig();
    config.repeats = static_cast<std::size_t>(
        io_.ReadIntOrDefault("Повторов построения (берётся медиана)",
                             static_cast<int>(config.repeats), 1, kMaxRepeats));
    config.seed = nextSeed_;

    io_.Out() << "Измеряю…\n";
    const ArraySequence<Measurement> measurements =
        CompareMethodsOnData(database_.GetPeople(), attribute, config);
    PrintMeasurements(measurements, false);
    io_.Out() << "Запросов: по ключу и по диапазону по " << config.queryCount
              << " (у линейного поиска по " << config.linearQueryCount << ").\n";
    OfferCsvExport(measurements);
}

// --- Эксперимент ------------------------------------------------------------

void ConsoleUI::RunExperimentMenu()
{
    ExperimentConfig config = MakeDefaultExperimentConfig();
    io_.Out() << "Эксперимент: для каждого размера генерируются одни и те же люди в трёх "
                 "порядках (по возрастанию ключа, по убыванию, случайно), и все методы "
                 "строят по ним индекс.\n"
              << "Совет: для честных замеров запускайте сборку Release.\n";

    const ArraySequence<int> sizes =
        io_.ReadIntList("Размеры через пробел (например, «1000 10000 100000»): ", 1,
                        kMaxExperimentSize);
    config.sizes = ArraySequence<std::size_t>();
    for (std::size_t i = 0; i < sizes.GetLength(); ++i)
    {
        config.sizes.Append(static_cast<std::size_t>(sizes.Get(i)));
    }
    if (!io_.ReadYesNo("Атрибуты по умолчанию («номер» и «фамилия + имя + год рождения»)?",
                       true))
    {
        config.attributes = ArraySequence<CompositeAttribute>{ReadCompositeAttribute()};
    }
    config.repeats = static_cast<std::size_t>(io_.ReadIntOrDefault(
        "Повторов построения", static_cast<int>(config.repeats), 1, kMaxRepeats));
    config.seed = static_cast<unsigned>(io_.ReadIntOrDefault(
        "Начальное значение генератора (seed)", static_cast<int>(config.seed), 0, INT_MAX));

    io_.Out() << "Измеряю…\n";
    std::string currentAttribute;
    const ArraySequence<Measurement> measurements = RunExperiment(
        config, [this, &currentAttribute](const Measurement& m)
        {
            if (m.attribute != currentAttribute)
            {
                currentAttribute = m.attribute;
                io_.Out() << "Атрибут: " << currentAttribute << "\n";
            }
            PrintMeasurements(ArraySequence<Measurement>{m}, true);
        });
    OfferCsvExport(measurements);
}

void ConsoleUI::PrintMeasurements(const ArraySequence<Measurement>& measurements, bool showOrder)
{
    for (std::size_t i = 0; i < measurements.GetLength(); ++i)
    {
        const Measurement& m = measurements.Get(i);
        io_.Out() << PadRight(std::to_string(m.size), kSizeWidth);
        if (showOrder)
        {
            io_.Out() << PadRight(GetOrderName(m.order), kOrderWidth);
        }
        io_.Out() << PadRight(GetMethodName(m.method), kMethodWidth);
        if (m.skipped)
        {
            io_.Out() << "пропущен: построение O(n²) на большом размере\n";
            continue;
        }
        io_.Out() << "построение " << PadLeft(FormatFixed(m.buildMs, kDigits), kValueWidth)
                  << " мс | по ключу " << PadLeft(FormatFixed(m.findUs, kDigits), kValueWidth)
                  << " мкс | диапазон " << PadLeft(FormatFixed(m.rangeUs, kDigits), kValueWidth)
                  << " мкс\n";
    }
}

void ConsoleUI::OfferCsvExport(const ArraySequence<Measurement>& measurements)
{
    if (!io_.ReadYesNo("Сохранить результаты в CSV?", false))
    {
        return;
    }
    std::string path = io_.ReadLine("Путь к файлу [results.csv]: ");
    if (path.empty())
    {
        path = "results.csv";
    }
    std::ofstream out(std::filesystem::u8path(path));
    if (!out)
    {
        throw FileError("Не удалось создать файл «" + path
                        + "»: проверьте, что папка существует и доступна для записи");
    }
    WriteMeasurementsCsv(measurements, out);
    io_.Out() << "Сохранено в «" << path << "». Графики: python bench/plot.py " << path << "\n";
}
