#include "PersonCsv.h"

#include <filesystem>
#include <fstream>

#include "Exceptions.h"
#include "TextUtils.h"

namespace
{

const char kSeparator = ';';
const char* const kHeader = "id;last_name;first_name;middle_name;birth_year";
// Порядок полей в строке CSV.
enum CsvField : std::size_t
{
    kIdField,
    kLastNameField,
    kFirstNameField,
    kMiddleNameField,
    kBirthYearField,
    kFieldCount
};

ArraySequence<std::string> SplitFields(const std::string& line)
{
    ArraySequence<std::string> fields;
    std::size_t begin = 0;
    while (true)
    {
        const std::size_t end = line.find(kSeparator, begin);
        if (end == std::string::npos)
        {
            fields.Append(Trim(line.substr(begin)));
            return fields;
        }
        fields.Append(Trim(line.substr(begin, end - begin)));
        begin = end + 1;
    }
}

Person ParsePerson(const std::string& line, std::size_t lineNumber)
{
    const ArraySequence<std::string> fields = SplitFields(line);
    try
    {
        if (fields.GetLength() != kFieldCount)
        {
            throw InvalidArgument("ожидается " + std::to_string(kFieldCount)
                                  + " полей через ';', найдено "
                                  + std::to_string(fields.GetLength()));
        }
        const int id = ParseInteger(fields.Get(kIdField), "номер");
        const int birthYear = ParseInteger(fields.Get(kBirthYearField), "год рождения");
        return Person(id, fields.Get(kLastNameField), fields.Get(kFirstNameField),
                      fields.Get(kMiddleNameField), birthYear);
    }
    catch (const InvalidArgument& error)
    {
        throw ParseError("Строка " + std::to_string(lineNumber) + ": " + error.what());
    }
}

// Путь из UTF-8 в путь файловой системы. В Windows это нужно, чтобы
// открывались файлы в папках с русскими буквами.
std::filesystem::path ToPath(const std::string& path)
{
    return std::filesystem::u8path(path);
}

} // namespace

void WritePeopleCsv(const ArraySequence<Person>& people, std::ostream& out)
{
    out << kHeader << '\n';
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        const Person& person = people.Get(i);
        out << person.GetId() << kSeparator << person.GetLastName() << kSeparator
            << person.GetFirstName() << kSeparator << person.GetMiddleName() << kSeparator
            << person.GetBirthYear() << '\n';
    }
}

ArraySequence<Person> ReadPeopleCsv(std::istream& in)
{
    ArraySequence<Person> people;
    std::string line;
    std::size_t lineNumber = 0;
    bool headerChecked = false;
    while (std::getline(in, line))
    {
        ++lineNumber;
        const std::string trimmed = Trim(line);
        if (trimmed.empty())
        {
            continue;
        }
        if (!headerChecked)
        {
            headerChecked = true;
            if (trimmed == kHeader)
            {
                continue;
            }
            // Заголовка нет — первая строка разбирается как запись.
        }
        people.Append(ParsePerson(trimmed, lineNumber));
    }
    if (in.bad())
    {
        throw FileError("Ошибка чтения данных на строке " + std::to_string(lineNumber + 1));
    }
    return people;
}

void SavePeopleToFile(const ArraySequence<Person>& people, const std::string& path)
{
    std::ofstream out(ToPath(path));
    if (!out)
    {
        throw FileError("Не удалось создать файл «" + path
                        + "»: проверьте, что папка существует и доступна для записи");
    }
    WritePeopleCsv(people, out);
    out.flush();
    if (!out)
    {
        throw FileError("Не удалось записать файл «" + path + "»: возможно, диск заполнен");
    }
}

ArraySequence<Person> LoadPeopleFromFile(const std::string& path)
{
    std::ifstream in(ToPath(path));
    if (!in)
    {
        throw FileError("Не удалось открыть файл «" + path
                        + "»: проверьте путь и что файл существует");
    }
    return ReadPeopleCsv(in);
}
