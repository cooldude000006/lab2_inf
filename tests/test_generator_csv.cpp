// Тесты генератора данных и CSV (DESIGN.md, п. 4.8).

#include <filesystem>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

#include "Attribute.h"
#include "Exceptions.h"
#include "PersonCsv.h"
#include "PersonGenerator.h"
#include "TestHelpers.h"

namespace
{

bool EndsWith(const std::string& text, const std::string& suffix)
{
    return text.size() >= suffix.size()
           && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string ExpectParseError(const std::string& csv)
{
    std::istringstream in(csv);
    try
    {
        ReadPeopleCsv(in);
    }
    catch (const ParseError& error)
    {
        return error.what();
    }
    ADD_FAILURE() << "ожидалась ParseError для:\n" << csv;
    return "";
}

} // namespace

// --- Генератор --------------------------------------------------------------

TEST(GeneratorTest, CountIdsAndYears)
{
    PersonGenerator generator(1);
    const ArraySequence<Person> people = generator.Generate(500, 1970, 1980);
    ASSERT_EQ(people.GetLength(), 500u);
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        const Person& person = people.Get(i);
        EXPECT_EQ(person.GetId(), static_cast<int>(i) + 1);
        EXPECT_GE(person.GetBirthYear(), 1970);
        EXPECT_LE(person.GetBirthYear(), 1980);
    }
}

TEST(GeneratorTest, SameSeedSameData)
{
    PersonGenerator first(42);
    PersonGenerator second(42);
    PersonGenerator other(43);
    const ArraySequence<Person> a = first.Generate(100, 1950, 2000);
    EXPECT_EQ(a, second.Generate(100, 1950, 2000));
    EXPECT_NE(a, other.Generate(100, 1950, 2000));
}

TEST(GeneratorTest, GenderIsConsistent)
{
    PersonGenerator generator(3);
    const ArraySequence<Person> people = generator.Generate(300, 1950, 2000);
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        const Person& person = people.Get(i);
        const bool femaleLastName = EndsWith(person.GetLastName(), "а");
        const bool femaleMiddleName = EndsWith(person.GetMiddleName(), "на");
        EXPECT_EQ(femaleLastName, femaleMiddleName) << person.GetFullName();
    }
}

TEST(GeneratorTest, OrderedByAttribute)
{
    const CompositeAttribute attribute(ArraySequence<Attribute>{
        Attribute(AttributeId::BirthYear), Attribute(AttributeId::LastName)});
    for (DataOrder order : {DataOrder::Ascending, DataOrder::Descending})
    {
        PersonGenerator generator(9);
        const ArraySequence<Person> people =
            generator.GenerateOrdered(400, 1950, 2000, attribute, order);
        ASSERT_EQ(people.GetLength(), 400u);
        for (std::size_t i = 1; i < people.GetLength(); ++i)
        {
            const CompositeKey previous = attribute.MakeKey(people.Get(i - 1));
            const CompositeKey current = attribute.MakeKey(people.Get(i));
            if (order == DataOrder::Ascending)
            {
                EXPECT_LE(previous, current);
            }
            else
            {
                EXPECT_GE(previous, current);
            }
        }
    }
}

TEST(GeneratorTest, RandomOrderIsShuffledPermutation)
{
    PersonGenerator first(5);
    PersonGenerator second(5);
    const CompositeAttribute byId(Attribute(AttributeId::Id));
    const std::size_t count = 200;
    const ArraySequence<Person> shuffled =
        first.GenerateOrdered(count, 1950, 2000, byId, DataOrder::Random);
    const ArraySequence<Person> plain = second.Generate(count, 1950, 2000);

    // Те же люди: на позиции (номер − 1) обычной генерации лежит та же запись.
    ASSERT_EQ(shuffled.GetLength(), count);
    std::size_t inPlace = 0;
    for (std::size_t i = 0; i < count; ++i)
    {
        const Person& person = shuffled.Get(i);
        EXPECT_EQ(person, plain.Get(static_cast<std::size_t>(person.GetId()) - 1));
        if (person.GetId() == static_cast<int>(i) + 1)
        {
            ++inPlace;
        }
    }
    // Перемешаны: на своих местах осталась лишь малая часть (в среднем одна запись).
    EXPECT_LT(inPlace, count / 10);
}

TEST(GeneratorTest, InvalidArguments)
{
    PersonGenerator generator(1);
    EXPECT_THROW(generator.Generate(10, 2000, 1990), InvalidArgument);
    EXPECT_THROW(generator.Generate(10, 1800, 1990), InvalidArgument);
    EXPECT_THROW(generator.Generate(10, 1990, CurrentYear() + 1), InvalidArgument);
    EXPECT_TRUE(generator.Generate(0, 1990, 2000).GetIsEmpty());
}

// --- CSV --------------------------------------------------------------------

TEST(CsvTest, RoundTrip)
{
    PersonGenerator generator(11);
    ArraySequence<Person> people = generator.Generate(50, 1950, 2000);
    people.Append(Person(51, "Smith", "John", "", 1980)); // пустое отчество

    std::stringstream stream;
    WritePeopleCsv(people, stream);
    EXPECT_EQ(ReadPeopleCsv(stream), people);
}

TEST(CsvTest, HeaderOptionalBlankLinesAndCrlf)
{
    std::istringstream withHeader("id;last_name;first_name;middle_name;birth_year\r\n"
                                  "\r\n"
                                  "1;Иванов;Пётр;Сергеевич;1995\r\n");
    std::istringstream withoutHeader(" 1 ; Иванов ; Пётр ; Сергеевич ; 1995 \n");
    const ArraySequence<Person> expected{Person(1, "Иванов", "Пётр", "Сергеевич", 1995)};
    EXPECT_EQ(ReadPeopleCsv(withHeader), expected);
    EXPECT_EQ(ReadPeopleCsv(withoutHeader), expected);
}

TEST(CsvTest, EmptyInput)
{
    std::istringstream empty("");
    EXPECT_TRUE(ReadPeopleCsv(empty).GetIsEmpty());
}

TEST(CsvTest, ErrorsReportLineNumber)
{
    const std::string header = "id;last_name;first_name;middle_name;birth_year\n";
    EXPECT_NE(ExpectParseError(header + "1;Иванов;Пётр;;1995\n2;Петров;Иван\n").find("Строка 3"),
              std::string::npos);
    EXPECT_NE(ExpectParseError(header + "x;Иванов;Пётр;;1995\n").find("Строка 2"),
              std::string::npos);
    EXPECT_NE(ExpectParseError(header + "1;Иванов;Пётр;;год\n").find("год рождения"),
              std::string::npos);
    EXPECT_NE(ExpectParseError(header + "1;;Пётр;;1995\n").find("Фамилия"), std::string::npos);
    EXPECT_NE(ExpectParseError(header + "1;Иванов;Пётр;;1800\n").find("Строка 2"),
              std::string::npos);
}

TEST(CsvTest, FileRoundTripWithCyrillicPath)
{
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / std::filesystem::u8path("лр2_тест");
    std::filesystem::create_directories(directory);
    const std::string path = (directory / std::filesystem::u8path("люди.csv")).u8string();

    PersonGenerator generator(13);
    const ArraySequence<Person> people = generator.Generate(20, 1950, 2000);
    SavePeopleToFile(people, path);
    EXPECT_EQ(LoadPeopleFromFile(path), people);

    std::filesystem::remove_all(directory);
}

TEST(CsvTest, MissingFileThrows)
{
    EXPECT_THROW(LoadPeopleFromFile("нет/такого/файла.csv"), FileError);
    EXPECT_THROW(SavePeopleToFile(ArraySequence<Person>(), "нет/такой/папки/файл.csv"), FileError);
}
