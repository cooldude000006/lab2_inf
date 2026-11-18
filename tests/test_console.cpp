// Тесты консольного ввода и сценариев интерфейса (DESIGN.md, п. 4.10).
// Задание не требует тестировать интерфейс; эти тесты проверяют, что
// некорректный ввод не ломает программу и основные сценарии доходят до конца.

#include <gtest/gtest.h>
#include <sstream>
#include <string>

#include "ConsoleIO.h"
#include "ConsoleUI.h"

namespace
{

// Запускает интерфейс на заданном вводе и возвращает весь вывод.
std::string RunSession(const std::string& input)
{
    std::istringstream in(input);
    std::ostringstream out;
    ConsoleIO io(in, out, false);
    ConsoleUI ui(io);
    ui.Run();
    return out.str();
}

bool Contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

} // namespace

// --- ConsoleIO --------------------------------------------------------------

TEST(ConsoleIOTest, ReadIntRepeatsUntilValid)
{
    std::istringstream in("abc\n99\n\n7\n");
    std::ostringstream out;
    ConsoleIO io(in, out, false);
    EXPECT_EQ(io.ReadInt("> ", 1, 10), 7);
    EXPECT_TRUE(Contains(out.str(), "не целое число"));
    EXPECT_TRUE(Contains(out.str(), "от 1 до 10"));
}

TEST(ConsoleIOTest, DefaultsAndYesNo)
{
    std::istringstream in("\n42\n\nн\nда\n");
    std::ostringstream out;
    ConsoleIO io(in, out, false);
    EXPECT_EQ(io.ReadIntOrDefault("a", 5, 0, 100), 5);
    EXPECT_EQ(io.ReadIntOrDefault("b", 5, 0, 100), 42);
    EXPECT_TRUE(io.ReadYesNo("c", true));
    EXPECT_FALSE(io.ReadYesNo("d", true));
    EXPECT_TRUE(io.ReadYesNo("e", false));
}

TEST(ConsoleIOTest, IntListWithSeparatorsAndRetry)
{
    std::istringstream in("\n2 x\n2, 3  7\n");
    std::ostringstream out;
    ConsoleIO io(in, out, false);
    const ArraySequence<int> values = io.ReadIntList("> ", 1, 8);
    EXPECT_EQ(values, (ArraySequence<int>{2, 3, 7}));
}

TEST(ConsoleIOTest, EndOfInputThrowsInputEnded)
{
    std::istringstream in("");
    std::ostringstream out;
    ConsoleIO io(in, out, false);
    EXPECT_THROW(io.ReadLine("> "), InputEnded);
}

// --- Сценарии интерфейса ----------------------------------------------------

TEST(ConsoleUITest, ExitAndEndOfInput)
{
    EXPECT_TRUE(Contains(RunSession("0\n"), "До свидания"));
    EXPECT_TRUE(Contains(RunSession(""), "Ввод завершён"));
}

TEST(ConsoleUITest, InvalidMenuInputDoesNotBreak)
{
    const std::string output = RunSession("abc\n42\n3\n0\n");
    EXPECT_TRUE(Contains(output, "не целое число"));
    EXPECT_TRUE(Contains(output, "от 0 до 5"));
    EXPECT_TRUE(Contains(output, "Индексов нет"));
    EXPECT_TRUE(Contains(output, "До свидания"));
}

TEST(ConsoleUITest, GenerateBuildIndexAndSearchByKey)
{
    // Данные → сгенерировать 300 (остальное по умолчанию);
    // Индексы → построить по номеру (1), АВЛ-дерево (3);
    // Поиск → индекс 1 → по ключу → номер 5; выход.
    const std::string output = RunSession("1\n1\n300\n\n\n\n\n"
                                          "2\n1\n1\n3\n"
                                          "3\n1\n1\n5\n"
                                          "0\n");
    EXPECT_TRUE(Contains(output, "Создано записей: 300"));
    EXPECT_TRUE(Contains(output, "АВЛ-дерево, ключей: 300"));
    EXPECT_TRUE(Contains(output, "Найдено записей: 1"));
    EXPECT_TRUE(Contains(output, "результаты совпадают"));
}

TEST(ConsoleUITest, RangeSearchWithCompositeKeyAndBadValue)
{
    // Индекс «фамилия + год рождения» бинарным поиском; диапазон
    // (А, 1900)–(я, 2100); в первом вводе года — не число.
    // Верхняя граница — строчная «я»: она больше любой фамилии с заглавной
    // буквы. С заглавной «Я» Яковлевы не попали бы в диапазон: «Я» — начало
    // строки «Яковлев», а начало строки меньше самой строки.
    const std::string output = RunSession("1\n1\n200\n\n\n\n\n"
                                          "2\n1\n2 7\n2\n"
                                          "3\n1\n2\nА\nгод\nА\n1900\nя\n2100\n"
                                          "0\n");
    EXPECT_TRUE(Contains(output, "не целое число"));
    EXPECT_TRUE(Contains(output, "Найдено записей: 200"));
    EXPECT_TRUE(Contains(output, "результаты совпадают"));
}

TEST(ConsoleUITest, ReversedRangeReportsError)
{
    const std::string output = RunSession("1\n1\n50\n\n\n\n\n"
                                          "2\n1\n7\n3\n"
                                          "3\n1\n2\n2000\n1990\n"
                                          "0\n");
    EXPECT_TRUE(Contains(output, "Ошибка: Верхняя граница диапазона"));
    EXPECT_TRUE(Contains(output, "До свидания"));
}

TEST(ConsoleUITest, AddPersonUpdatesIndex)
{
    // Индекс по фамилии над пустой базой, затем ручное добавление и поиск.
    const std::string output = RunSession("2\n1\n2\n2\n"
                                          "1\n2\n\nИванов\nПётр\n\n1995\n"
                                          "3\n1\n1\nИванов\n"
                                          "0\n");
    EXPECT_TRUE(Contains(output, "Добавлено на позицию 1"));
    EXPECT_TRUE(Contains(output, "Найдено записей: 1"));
    EXPECT_TRUE(Contains(output, "Иванов Пётр"));
}

TEST(ConsoleUITest, CompareMethodsPrintsAllMethods)
{
    const std::string output = RunSession("1\n1\n500\n\n\n\n\n"
                                          "4\n1\n1\n\n"
                                          "0\n");
    EXPECT_TRUE(Contains(output, "линейный поиск"));
    EXPECT_TRUE(Contains(output, "бинарный поиск"));
    EXPECT_TRUE(Contains(output, "АВЛ-дерево"));
    EXPECT_TRUE(Contains(output, "построение"));
}

TEST(ConsoleUITest, MissingFileReportsError)
{
    const std::string output = RunSession("1\n3\nнет/такого/файла.csv\n0\n");
    EXPECT_TRUE(Contains(output, "Ошибка: Не удалось открыть файл"));
}
