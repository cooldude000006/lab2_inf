#include "PersonGenerator.h"

#include <climits>

#include "CompositeKey.h"
#include "Exceptions.h"
#include "MergeSort.h"
#include "Pair.h"

namespace
{

// Фамилии в мужской форме; женская получается добавлением «а»
// (все фамилии списка оканчиваются на -ов, -ев или -ин).
const char* const kLastNames[] = {
    "Иванов",   "Смирнов",  "Кузнецов", "Попов",    "Васильев", "Петров",   "Соколов",
    "Михайлов", "Новиков",  "Фёдоров",  "Морозов",  "Волков",   "Алексеев", "Лебедев",
    "Семёнов",  "Егоров",   "Павлов",   "Козлов",   "Степанов", "Николаев", "Орлов",
    "Андреев",  "Макаров",  "Никитин",  "Захаров",  "Зайцев",   "Соловьёв", "Борисов",
    "Яковлев",  "Григорьев", "Романов", "Воробьёв", "Сергеев",  "Кузьмин",  "Фролов",
    "Александров", "Дмитриев", "Королёв", "Гусев",  "Киселёв",  "Ильин",    "Максимов",
    "Поляков",  "Сорокин",  "Виноградов", "Ковалёв", "Белов",   "Медведев", "Антонов",
    "Тарасов"};

const char* const kMaleFirstNames[] = {
    "Александр", "Алексей", "Андрей",  "Артём",   "Борис",   "Вадим",   "Валерий",
    "Василий",   "Виктор",  "Владимир", "Глеб",   "Григорий", "Даниил", "Денис",
    "Дмитрий",   "Евгений", "Егор",    "Иван",    "Игорь",   "Илья",    "Кирилл",
    "Константин", "Лев",    "Максим",  "Михаил",  "Никита",  "Николай", "Олег",
    "Павел",     "Пётр",    "Роман",   "Сергей",  "Степан",  "Тимофей", "Фёдор",
    "Юрий"};

const char* const kFemaleFirstNames[] = {
    "Александра", "Алина",  "Алла",    "Анастасия", "Анна",   "Валентина", "Валерия",
    "Вера",       "Виктория", "Галина", "Дарья",    "Евгения", "Екатерина", "Елена",
    "Елизавета",  "Ирина",  "Ксения",  "Лариса",    "Любовь", "Людмила",   "Маргарита",
    "Марина",     "Мария",  "Надежда", "Наталья",   "Нина",   "Оксана",    "Ольга",
    "Полина",     "Светлана", "София", "Татьяна",   "Ульяна", "Юлия"};

// Отчества: мужская и женская формы на одной позиции.
const char* const kMaleMiddleNames[] = {
    "Александрович", "Алексеевич",  "Андреевич", "Борисович",  "Васильевич",
    "Викторович",    "Владимирович", "Дмитриевич", "Евгеньевич", "Иванович",
    "Игоревич",      "Михайлович",  "Николаевич", "Олегович",  "Павлович",
    "Петрович",      "Сергеевич",   "Юрьевич"};

const char* const kFemaleMiddleNames[] = {
    "Александровна", "Алексеевна",  "Андреевна", "Борисовна",  "Васильевна",
    "Викторовна",    "Владимировна", "Дмитриевна", "Евгеньевна", "Ивановна",
    "Игоревна",      "Михайловна",  "Николаевна", "Олеговна",  "Павловна",
    "Петровна",      "Сергеевна",   "Юрьевна"};

template <class T, std::size_t N>
constexpr std::size_t CountOf(const T (&)[N])
{
    return N;
}

using KeyedPerson = Pair<CompositeKey, Person>;

// Сравнение записей только по ключу (для упорядочивания генератором).
class KeyedPersonLess
{
public:
    bool operator()(const KeyedPerson& left, const KeyedPerson& right) const;
};

bool KeyedPersonLess::operator()(const KeyedPerson& left, const KeyedPerson& right) const
{
    return left.key < right.key;
}

} // namespace

std::string GetOrderName(DataOrder order)
{
    switch (order)
    {
        case DataOrder::Ascending:
            return "по возрастанию";
        case DataOrder::Descending:
            return "по убыванию";
        case DataOrder::Random:
            return "случайный";
        case DataOrder::AsIs:
            return "как есть";
    }
    return "?";
}

ArraySequence<DataOrder> GetAllDataOrders()
{
    return ArraySequence<DataOrder>{DataOrder::Ascending, DataOrder::Descending,
                                    DataOrder::Random};
}

PersonGenerator::PersonGenerator(unsigned seed)
    : random_(seed)
{
}

ArraySequence<Person> PersonGenerator::Generate(std::size_t count, int minYear, int maxYear)
{
    if (minYear > maxYear)
    {
        throw InvalidArgument("Начальный год (" + std::to_string(minYear)
                              + ") больше конечного (" + std::to_string(maxYear)
                              + "); поменяйте их местами");
    }
    if (minYear < Person::kMinBirthYear || maxYear > CurrentYear())
    {
        throw InvalidArgument("Годы рождения должны быть от "
                              + std::to_string(Person::kMinBirthYear) + " до "
                              + std::to_string(CurrentYear()));
    }
    if (count > static_cast<std::size_t>(INT_MAX))
    {
        throw InvalidArgument("Слишком много записей: не больше " + std::to_string(INT_MAX));
    }

    ArraySequence<Person> people;
    people.Reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        people.Append(MakePerson(static_cast<int>(i) + 1, minYear, maxYear));
    }
    return people;
}

ArraySequence<Person> PersonGenerator::GenerateOrdered(std::size_t count, int minYear,
                                                       int maxYear,
                                                       const CompositeAttribute& attribute,
                                                       DataOrder order)
{
    ArraySequence<Person> people = Generate(count, minYear, maxYear);
    if (order == DataOrder::AsIs)
    {
        return people;
    }
    if (order == DataOrder::Random)
    {
        // Порядок создания не случаен по номеру (номера идут 1, 2, ...),
        // поэтому записи перемешиваются: тасование Фишера–Йетса,
        // все перестановки равновероятны.
        for (std::size_t i = people.GetLength(); i > 1; --i)
        {
            const std::size_t j = RandomIndex(i);
            const Person swapped = people.Get(i - 1);
            people.Set(i - 1, people.Get(j));
            people.Set(j, swapped);
        }
        return people;
    }

    // Ключ каждой записи вычисляется один раз, затем записи раскладываются
    // по ключу устойчивой сортировкой.
    ArraySequence<KeyedPerson> keyed;
    keyed.Reserve(count);
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        keyed.Append(KeyedPerson{attribute.MakeKey(people.Get(i)), people.Get(i)});
    }
    MergeSort(keyed, KeyedPersonLess());

    // Для убывания — обход с конца; равные ключи при этом тоже
    // окажутся в обратном порядке создания, на замеры это не влияет.
    for (std::size_t i = 0; i < keyed.GetLength(); ++i)
    {
        const std::size_t source = order == DataOrder::Ascending ? i : keyed.GetLength() - 1 - i;
        people.Set(i, keyed.Get(source).value);
    }
    return people;
}

Person PersonGenerator::MakePerson(int id, int minYear, int maxYear)
{
    std::uniform_int_distribution<int> yearDistribution(minYear, maxYear);
    std::bernoulli_distribution isFemale(0.5);

    std::string lastName = kLastNames[RandomIndex(CountOf(kLastNames))];
    const std::size_t middle = RandomIndex(CountOf(kMaleMiddleNames));
    std::string firstName;
    std::string middleName;
    if (isFemale(random_))
    {
        lastName += "а";
        firstName = kFemaleFirstNames[RandomIndex(CountOf(kFemaleFirstNames))];
        middleName = kFemaleMiddleNames[middle];
    }
    else
    {
        firstName = kMaleFirstNames[RandomIndex(CountOf(kMaleFirstNames))];
        middleName = kMaleMiddleNames[middle];
    }
    return Person(id, lastName, firstName, middleName, yearDistribution(random_));
}

std::size_t PersonGenerator::RandomIndex(std::size_t size)
{
    std::uniform_int_distribution<std::size_t> distribution(0, size - 1);
    return distribution(random_);
}
