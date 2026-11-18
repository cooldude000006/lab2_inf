// Консольный интерфейс: главное меню, данные, индексы, выбор и вывод.
// Поиск, сравнение и эксперимент — в ConsoleUISearch.cpp.

#include "ConsoleUI.h"

#include <chrono>
#include <climits>
#include <new>

#include "Exceptions.h"
#include "LinearSearch.h"
#include "PersonCsv.h"
#include "TextUtils.h"

namespace
{

using Clock = std::chrono::steady_clock;

const int kMaxGeneratedPeople = 5000000;
const int kDefaultPeopleCount = 1000;
const int kDefaultMinYear = 1950;
const int kDefaultMaxYear = 2005;
const unsigned kFirstSeed = 2024;
const int kDefaultPageSize = 20;
const int kMaxPageSize = 1000;
const int kShownPositions = 5;
const std::size_t kSlowBuildSize = 100000;
const int kMaxYearsAhead = 100; // возраст можно считать на годы вперёд

// Ширина столбцов таблицы людей в символах.
const std::size_t kPositionWidth = 9;
const std::size_t kIdWidth = 9;
const std::size_t kNameWidth = 40;

double MillisecondsSince(Clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

} // namespace

ConsoleUI::ConsoleUI(ConsoleIO& io)
    : io_(io), database_(), nextSeed_(kFirstSeed)
{
}

// --- Главное меню -----------------------------------------------------------

void ConsoleUI::Run()
{
    io_.Out() << "ЛР-2: индексирование данных и сравнение алгоритмов поиска\n"
              << "Методы: линейный поиск, бинарный поиск по отсортированной "
                 "последовательности, АВЛ-дерево.\n"
              << "Начните с пункта «Данные» (сгенерировать или загрузить людей), "
                 "затем постройте индекс.\n";
    try
    {
        while (RunMainMenuOnce())
        {
        }
    }
    catch (const InputEnded&)
    {
        io_.Out() << "Ввод завершён.\n";
    }
    io_.Out() << "До свидания!\n";
}

// Один проход главного меню. Возвращает false, когда пользователь выбрал выход.
// Ошибки действий перехватываются здесь, чтобы программа продолжала работу.
bool ConsoleUI::RunMainMenuOnce()
{
    PrintStatus();
    const int choice = io_.ReadMenuChoice(
        "Главное меню",
        ArraySequence<std::string>{"Данные", "Индексы", "Поиск",
                                   "Сравнить методы на текущих данных",
                                   "Эксперимент на сгенерированных данных (замеры, CSV)"},
        "Выход");
    try
    {
        switch (choice)
        {
            case 0:
                return false;
            case 1:
                DataMenu();
                break;
            case 2:
                IndexMenu();
                break;
            case 3:
                SearchMenu();
                break;
            case 4:
                CompareMethods();
                break;
            case 5:
                RunExperimentMenu();
                break;
            default:
                break;
        }
    }
    catch (const Lab2Error& error)
    {
        io_.Out() << "Ошибка: " << error.what() << "\n";
    }
    catch (const std::bad_alloc&)
    {
        io_.Out() << "Ошибка: не хватает памяти; уменьшите число записей.\n";
    }
    return true;
}

void ConsoleUI::DataMenu()
{
    const int choice = io_.ReadMenuChoice(
        "Данные",
        ArraySequence<std::string>{"Сгенерировать людей", "Добавить человека вручную",
                                   "Загрузить из CSV", "Сохранить в CSV", "Показать записи"},
        "Назад");
    switch (choice)
    {
        case 1:
            GenerateData();
            break;
        case 2:
            AddPersonManually();
            break;
        case 3:
            LoadFromFile();
            break;
        case 4:
            SaveToFile();
            break;
        case 5:
            ShowPeople();
            break;
        default:
            break;
    }
}

void ConsoleUI::IndexMenu()
{
    const int choice = io_.ReadMenuChoice(
        "Индексы",
        ArraySequence<std::string>{"Построить индекс", "Список индексов",
                                   "Показать содержимое индекса", "Удалить индекс"},
        "Назад");
    switch (choice)
    {
        case 1:
            CreateIndex();
            break;
        case 2:
            ListIndexes();
            break;
        case 3:
            ShowIndexEntries();
            break;
        case 4:
            RemoveIndex();
            break;
        default:
            break;
    }
}

// --- Данные -----------------------------------------------------------------

void ConsoleUI::GenerateData()
{
    const int count = io_.ReadIntOrDefault("Сколько людей создать", kDefaultPeopleCount, 0,
                                           kMaxGeneratedPeople);
    const int currentYear = CurrentYear();
    const int minYear = io_.ReadIntOrDefault("Самый ранний год рождения", kDefaultMinYear,
                                             Person::kMinBirthYear, currentYear);
    const int maxYear = io_.ReadIntOrDefault("Самый поздний год рождения",
                                             minYear > kDefaultMaxYear ? currentYear
                                                                       : kDefaultMaxYear,
                                             minYear, currentYear);
    const int seed = io_.ReadIntOrDefault("Начальное значение генератора (seed)",
                                          static_cast<int>(nextSeed_), 0, INT_MAX);
    const DataOrder order = ReadGenerationOrder();
    CompositeAttribute attribute;
    if (order == DataOrder::Ascending || order == DataOrder::Descending)
    {
        io_.Out() << "По какому атрибуту упорядочить записи?\n";
        attribute = ReadCompositeAttribute();
    }

    const Clock::time_point start = Clock::now();
    PersonGenerator generator(static_cast<unsigned>(seed));
    const ArraySequence<Person> people = generator.GenerateOrdered(
        static_cast<std::size_t>(count), minYear, maxYear, attribute, order);
    database_.ReplacePeople(people);
    nextSeed_ = static_cast<unsigned>(seed) + 1;

    io_.Out() << "Создано записей: " << people.GetLength() << " за "
              << FormatFixed(MillisecondsSince(start), 1) << " мс";
    if (database_.GetIndexCount() > 0)
    {
        io_.Out() << "; индексы перестроены: " << database_.GetIndexCount();
    }
    io_.Out() << ".\n";
}

void ConsoleUI::AddPersonManually()
{
    io_.Out() << "Новая запись (отчество можно оставить пустым).\n";
    const int id = io_.ReadIntOrDefault("Номер", database_.GetNextId(), 1, INT_MAX);
    const std::string lastName = io_.ReadNonEmptyLine("Фамилия: ");
    const std::string firstName = io_.ReadNonEmptyLine("Имя: ");
    const std::string middleName = io_.ReadLine("Отчество: ");
    const int birthYear = io_.ReadInt("Год рождения: ", Person::kMinBirthYear, CurrentYear());

    const std::size_t position =
        database_.AddPerson(Person(id, lastName, firstName, middleName, birthYear));
    io_.Out() << "Добавлено на позицию " << position + 1 << "; индексы обновлены.\n";
}

void ConsoleUI::LoadFromFile()
{
    const std::string path = io_.ReadNonEmptyLine("Путь к CSV-файлу: ");
    const Clock::time_point start = Clock::now();
    database_.ReplacePeople(LoadPeopleFromFile(path));
    io_.Out() << "Загружено записей: " << database_.GetCount() << " за "
              << FormatFixed(MillisecondsSince(start), 1) << " мс.\n";
}

void ConsoleUI::SaveToFile()
{
    std::string path = io_.ReadLine("Путь к CSV-файлу [people.csv]: ");
    if (path.empty())
    {
        path = "people.csv";
    }
    SavePeopleToFile(database_.GetPeople(), path);
    io_.Out() << "Сохранено записей: " << database_.GetCount() << " в «" << path << "».\n";
}

void ConsoleUI::ShowPeople()
{
    const std::size_t total = database_.GetCount();
    if (total == 0)
    {
        io_.Out() << "Записей нет. Сгенерируйте или загрузите данные.\n";
        return;
    }
    const int first = io_.ReadIntOrDefault("С какой позиции показать", 1, 1,
                                           static_cast<int>(total));
    const int count = io_.ReadIntOrDefault("Сколько записей показать", kDefaultPageSize, 1,
                                           kMaxPageSize);
    PrintPeopleHeader();
    const std::size_t begin = static_cast<std::size_t>(first) - 1;
    for (std::size_t i = begin; i < total && i < begin + static_cast<std::size_t>(count); ++i)
    {
        PrintPerson(i);
    }
}

// --- Индексы ----------------------------------------------------------------

void ConsoleUI::CreateIndex()
{
    const CompositeAttribute attribute = ReadCompositeAttribute();
    const SearchMethod method = ReadMethod();
    if (!ConfirmSlowBuild(attribute, method))
    {
        return;
    }
    const Clock::time_point start = Clock::now();
    const std::size_t number = database_.CreateIndex(attribute, method);
    io_.Out() << "Индекс " << number + 1 << " построен за "
              << FormatFixed(MillisecondsSince(start), 3) << " мс: " << DescribeIndex(number)
              << ".\n";
}

// Отсортированная последовательность на неупорядоченных данных строится
// за O(n²): на сотнях тысяч записей это минуты. Пользователь решает сам.
bool ConsoleUI::ConfirmSlowBuild(const CompositeAttribute& attribute, SearchMethod method)
{
    if (method != SearchMethod::BinarySearch || database_.GetCount() <= kSlowBuildSize
        || IsSortedByKey(database_.GetPeople(), attribute))
    {
        return true;
    }
    io_.Out() << "Внимание: записи не упорядочены по этому ключу, а их "
              << database_.GetCount() << ". Отсортированная последовательность будет "
                 "строиться за O(n²) — это может занять минуты.\n";
    return io_.ReadYesNo("Всё равно строить?", false);
}

void ConsoleUI::ListIndexes()
{
    if (database_.GetIndexCount() == 0)
    {
        io_.Out() << "Индексов нет. Постройте индекс в меню «Индексы».\n";
        return;
    }
    for (std::size_t i = 0; i < database_.GetIndexCount(); ++i)
    {
        io_.Out() << "  " << i + 1 << ". " << DescribeIndex(i) << "\n";
    }
}

void ConsoleUI::ShowIndexEntries()
{
    std::size_t number = 0;
    if (!ChooseIndex(number))
    {
        return;
    }
    const Index& index = database_.GetIndex(number);
    if (index.GetMethod() == SearchMethod::Linear)
    {
        io_.Out() << "У линейного поиска нет индекса: он каждый раз перебирает записи.\n";
        return;
    }
    const auto entries = index.GetEntries();
    const int count = io_.ReadIntOrDefault("Сколько ключей показать", kDefaultPageSize, 1,
                                           kMaxPageSize);
    io_.Out() << "Ключ → позиции записей (всего ключей: " << entries.GetLength() << ")\n";
    for (std::size_t i = 0; i < entries.GetLength() && i < static_cast<std::size_t>(count); ++i)
    {
        const Index::Positions& positions = entries.Get(i).value;
        io_.Out() << "  " << entries.Get(i).key.ToString() << " →";
        for (std::size_t j = 0; j < positions.GetLength() && j < kShownPositions; ++j)
        {
            io_.Out() << " " << positions.Get(j) + 1;
        }
        if (positions.GetLength() > kShownPositions)
        {
            io_.Out() << " … (всего " << positions.GetLength() << ")";
        }
        io_.Out() << "\n";
    }
}

void ConsoleUI::RemoveIndex()
{
    std::size_t number = 0;
    if (ChooseIndex(number))
    {
        database_.RemoveIndex(number);
        io_.Out() << "Индекс удалён.\n";
    }
}

// --- Выбор и ввод -----------------------------------------------------------

CompositeAttribute ConsoleUI::ReadCompositeAttribute()
{
    const ArraySequence<Attribute> plain = Attribute::GetPlainAttributes();
    const int ageChoice = static_cast<int>(plain.GetLength()) + 1;
    io_.Out() << "Атрибуты:\n";
    for (std::size_t i = 0; i < plain.GetLength(); ++i)
    {
        io_.Out() << "  " << i + 1 << ". " << plain.Get(i).GetName() << "\n";
    }
    io_.Out() << "  " << ageChoice << ". возраст на заданный год\n";

    while (true)
    {
        const ArraySequence<int> numbers = io_.ReadIntList(
            "Номера атрибутов через пробел (несколько — составной ключ, например «2 3 7»): ",
            1, ageChoice);
        ArraySequence<Attribute> parts;
        for (std::size_t i = 0; i < numbers.GetLength(); ++i)
        {
            if (numbers.Get(i) == ageChoice)
            {
                const int year = io_.ReadIntOrDefault("Год, на который считать возраст",
                                                      CurrentYear(), Person::kMinBirthYear,
                                                      CurrentYear() + kMaxYearsAhead);
                parts.Append(Attribute::AgeAt(year));
            }
            else
            {
                parts.Append(plain.Get(static_cast<std::size_t>(numbers.Get(i)) - 1));
            }
        }
        try
        {
            return CompositeAttribute(parts);
        }
        catch (const InvalidArgument& error)
        {
            io_.Out() << "  ! " << error.what() << "\n";
        }
    }
}

SearchMethod ConsoleUI::ReadMethod()
{
    const ArraySequence<SearchMethod> methods = GetAllSearchMethods();
    io_.Out() << "Метод поиска:\n";
    for (std::size_t i = 0; i < methods.GetLength(); ++i)
    {
        io_.Out() << "  " << i + 1 << ". " << GetMethodName(methods.Get(i)) << "\n";
    }
    const int choice = io_.ReadInt("Выберите метод: ", 1, static_cast<int>(methods.GetLength()));
    return methods.Get(static_cast<std::size_t>(choice) - 1);
}

DataOrder ConsoleUI::ReadGenerationOrder()
{
    io_.Out() << "Порядок записей:\n"
              << "  1. в порядке создания (номера по возрастанию)\n"
              << "  2. по возрастанию выбранного атрибута\n"
              << "  3. по убыванию выбранного атрибута\n"
              << "  4. случайно перемешать\n";
    const ArraySequence<DataOrder> orders{DataOrder::AsIs, DataOrder::Ascending,
                                          DataOrder::Descending, DataOrder::Random};
    const int choice = io_.ReadIntOrDefault("Выберите порядок", 1, 1,
                                            static_cast<int>(orders.GetLength()));
    return orders.Get(static_cast<std::size_t>(choice) - 1);
}

bool ConsoleUI::ChooseIndex(std::size_t& number)
{
    if (database_.GetIndexCount() == 0)
    {
        io_.Out() << "Индексов нет. Постройте индекс в меню «Индексы».\n";
        return false;
    }
    ListIndexes();
    const int choice = io_.ReadInt("Номер индекса: ", 1,
                                   static_cast<int>(database_.GetIndexCount()));
    number = static_cast<std::size_t>(choice) - 1;
    return true;
}

CompositeKey ConsoleUI::ReadKey(const CompositeAttribute& attribute, const std::string& title)
{
    while (true)
    {
        io_.Out() << title << ":\n";
        ArraySequence<std::string> texts;
        for (std::size_t i = 0; i < attribute.GetSize(); ++i)
        {
            texts.Append(io_.ReadNonEmptyLine("  " + attribute.Get(i).GetName() + ": "));
        }
        try
        {
            return attribute.ParseKey(texts);
        }
        catch (const InvalidArgument& error)
        {
            io_.Out() << "  ! " << error.what() << "\n";
        }
    }
}

// --- Вывод ------------------------------------------------------------------

void ConsoleUI::PrintStatus()
{
    io_.Out() << "\n[Записей: " << database_.GetCount()
              << ", индексов: " << database_.GetIndexCount() << "]\n";
}

void ConsoleUI::PrintPeopleHeader()
{
    io_.Out() << PadRight("позиция", kPositionWidth) << PadRight("номер", kIdWidth)
              << PadRight("ФИО", kNameWidth) << "год рождения\n";
}

void ConsoleUI::PrintPerson(std::size_t position)
{
    const Person& person = database_.GetPeople().Get(position);
    io_.Out() << PadRight(std::to_string(position + 1), kPositionWidth)
              << PadRight(std::to_string(person.GetId()), kIdWidth)
              << PadRight(person.GetFullName(), kNameWidth) << person.GetBirthYear() << "\n";
}

std::string ConsoleUI::DescribeIndex(std::size_t number) const
{
    const Index& index = database_.GetIndex(number);
    std::string text =
        "«" + index.GetAttribute().GetName() + "», " + GetMethodName(index.GetMethod());
    if (index.GetMethod() != SearchMethod::Linear)
    {
        text += ", ключей: " + std::to_string(index.GetKeyCount());
    }
    return text;
}
