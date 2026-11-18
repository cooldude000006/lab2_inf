// Тесты индекса и базы (DESIGN.md, п. 4.6, 4.7).
//
// Каждый тест индекса запускается для всех трёх методов поиска.
// Главная проверка — совпадение с линейным поиском на сгенерированных
// данных: по всем ключам, по диапазонам и по составным ключам.

#include <gtest/gtest.h>
#include <string>

#include "Attribute.h"
#include "Database.h"
#include "Exceptions.h"
#include "Index.h"
#include "LinearSearch.h"
#include "PersonGenerator.h"
#include "TestHelpers.h"

namespace
{

using Positions = Index::Positions;

const unsigned kSeed = 17;

CompositeAttribute LastNameAttribute()
{
    return CompositeAttribute(Attribute(AttributeId::LastName));
}

CompositeAttribute BirthYearAttribute()
{
    return CompositeAttribute(Attribute(AttributeId::BirthYear));
}

CompositeAttribute LastNameAndYearAttribute()
{
    return CompositeAttribute(ArraySequence<Attribute>{Attribute(AttributeId::LastName),
                                                       Attribute(AttributeId::BirthYear)});
}

CompositeKey YearKey(int year)
{
    return CompositeKey(ArraySequence<AttributeValue>{AttributeValue(year)});
}

class IndexTest : public ::testing::TestWithParam<SearchMethod>
{
protected:
    void SetUp() override
    {
        PersonGenerator generator(kSeed);
        people_ = generator.Generate(2000, 1950, 2005);
    }

    ArraySequence<Person> people_;
};

std::string MethodTestName(const ::testing::TestParamInfo<SearchMethod>& info)
{
    switch (info.param)
    {
        case SearchMethod::Linear:
            return "Linear";
        case SearchMethod::BinarySearch:
            return "BinarySearch";
        case SearchMethod::Avl:
            return "Avl";
    }
    return "Unknown";
}

} // namespace

INSTANTIATE_TEST_SUITE_P(AllMethods, IndexTest,
                         ::testing::Values(SearchMethod::Linear, SearchMethod::BinarySearch,
                                           SearchMethod::Avl),
                         MethodTestName);

// --- Совпадение с линейным поиском ------------------------------------------

TEST_P(IndexTest, FindMatchesLinearForEveryPersonKey)
{
    const CompositeAttribute attribute = LastNameAndYearAttribute();
    const Index index(&people_, attribute, GetParam());
    for (std::size_t i = 0; i < people_.GetLength(); i += 37)
    {
        const CompositeKey key = attribute.MakeKey(people_.Get(i));
        const Positions found = index.Find(key);
        EXPECT_EQ(found, LinearFind(people_, attribute, key));
        // Сама запись среди найденных.
        bool containsSelf = false;
        for (std::size_t j = 0; j < found.GetLength(); ++j)
        {
            containsSelf = containsSelf || found.Get(j) == i;
        }
        EXPECT_TRUE(containsSelf);
    }
}

TEST_P(IndexTest, RangeMatchesLinear)
{
    const CompositeAttribute attribute = BirthYearAttribute();
    const Index index(&people_, attribute, GetParam());
    for (int low = 1940; low <= 2010; low += 9)
    {
        for (int width : {0, 1, 5, 30})
        {
            const CompositeKey lowKey = YearKey(low);
            const CompositeKey highKey = YearKey(low + width);
            EXPECT_EQ(index.FindRange(lowKey, highKey),
                      LinearFindRange(people_, attribute, lowKey, highKey))
                << "диапазон " << low << "–" << low + width;
        }
    }
}

TEST_P(IndexTest, StringRangeMatchesLinear)
{
    const CompositeAttribute attribute = LastNameAttribute();
    const Index index(&people_, attribute, GetParam());
    const CompositeKey low = attribute.ParseKey(ArraySequence<std::string>{"К"});
    const CompositeKey high = attribute.ParseKey(ArraySequence<std::string>{"П"});
    const Positions found = index.FindRange(low, high);
    EXPECT_FALSE(found.GetIsEmpty());
    EXPECT_EQ(found, LinearFindRange(people_, attribute, low, high));
}

TEST_P(IndexTest, ResultIsSortedByPosition)
{
    const Index index(&people_, BirthYearAttribute(), GetParam());
    const Positions found = index.FindRange(YearKey(1960), YearKey(1990));
    for (std::size_t i = 1; i < found.GetLength(); ++i)
    {
        EXPECT_LT(found.Get(i - 1), found.Get(i));
    }
}

// --- Особые сценарии --------------------------------------------------------

TEST_P(IndexTest, MissingKeyGivesEmptyResult)
{
    const Index index(&people_, BirthYearAttribute(), GetParam());
    EXPECT_TRUE(index.Find(YearKey(1900)).GetIsEmpty());
    EXPECT_TRUE(index.FindRange(YearKey(1900), YearKey(1949)).GetIsEmpty());
}

TEST_P(IndexTest, EmptyCollection)
{
    const ArraySequence<Person> empty;
    const Index index(&empty, BirthYearAttribute(), GetParam());
    EXPECT_EQ(index.GetKeyCount(), 0u);
    EXPECT_TRUE(index.Find(YearKey(2000)).GetIsEmpty());
    EXPECT_TRUE(index.FindRange(YearKey(1900), YearKey(2020)).GetIsEmpty());
    EXPECT_TRUE(index.GetEntries().GetIsEmpty());
}

TEST_P(IndexTest, AllRecordsWithSameKey)
{
    ArraySequence<Person> same;
    for (int i = 1; i <= 5; ++i)
    {
        same.Append(Person(i, "Иванов", "Пётр", "", 2000));
    }
    const Index index(&same, BirthYearAttribute(), GetParam());
    EXPECT_EQ(index.Find(YearKey(2000)), (Positions{0, 1, 2, 3, 4}));
}

TEST_P(IndexTest, WrongKeyAndReversedRangeThrow)
{
    const Index index(&people_, BirthYearAttribute(), GetParam());
    const CompositeKey textKey(ArraySequence<AttributeValue>{AttributeValue(std::string("1995"))});
    EXPECT_THROW(index.Find(textKey), InvalidArgument);
    EXPECT_THROW(index.Find(CompositeKey()), InvalidArgument);
    EXPECT_THROW(index.FindRange(YearKey(2000), YearKey(1990)), InvalidArgument);
}

TEST_P(IndexTest, AgeBeforeBirthYearFails)
{
    const CompositeAttribute age(Attribute::AgeAt(1960));
    if (GetParam() == SearchMethod::Linear)
    {
        // Линейный метод ничего не строит: ошибка проявится при поиске.
        const Index index(&people_, age, GetParam());
        EXPECT_THROW(index.Find(YearKey(5)), InvalidArgument);
    }
    else
    {
        EXPECT_THROW(Index(&people_, age, GetParam()), InvalidArgument);
    }
}

TEST_P(IndexTest, NullCollectionThrows)
{
    EXPECT_THROW(Index(nullptr, BirthYearAttribute(), GetParam()), InvalidArgument);
}

TEST_P(IndexTest, AddPositionUpdatesIndex)
{
    ArraySequence<Person> people{Person(1, "Иванов", "Пётр", "", 2000)};
    Index index(&people, BirthYearAttribute(), GetParam());
    people.Append(Person(2, "Петров", "Иван", "", 2000));
    people.Append(Person(3, "Сидоров", "Олег", "", 1990));
    index.AddPosition(1);
    index.AddPosition(2);
    EXPECT_EQ(index.Find(YearKey(2000)), (Positions{0, 1}));
    EXPECT_EQ(index.Find(YearKey(1990)), (Positions{2}));
    EXPECT_THROW(index.AddPosition(3), InvalidArgument);
}

TEST_P(IndexTest, EntriesListAllKeys)
{
    const Index index(&people_, BirthYearAttribute(), GetParam());
    const auto entries = index.GetEntries();
    if (GetParam() == SearchMethod::Linear)
    {
        EXPECT_TRUE(entries.GetIsEmpty());
        return;
    }
    EXPECT_EQ(entries.GetLength(), index.GetKeyCount());
    std::size_t total = 0;
    for (std::size_t i = 0; i < entries.GetLength(); ++i)
    {
        if (i > 0)
        {
            EXPECT_LT(entries.Get(i - 1).key, entries.Get(i).key);
        }
        total += entries.Get(i).value.GetLength();
    }
    EXPECT_EQ(total, people_.GetLength());
}

// --- База -------------------------------------------------------------------

TEST(DatabaseTest, AddPersonUpdatesAllIndexes)
{
    Database database;
    database.CreateIndex(BirthYearAttribute(), SearchMethod::BinarySearch);
    database.CreateIndex(LastNameAttribute(), SearchMethod::Avl);
    EXPECT_EQ(database.GetNextId(), 1);

    database.AddPerson(Person(database.GetNextId(), "Иванов", "Пётр", "", 2000));
    database.AddPerson(Person(database.GetNextId(), "Петров", "Иван", "", 2000));
    EXPECT_EQ(database.GetCount(), 2u);
    EXPECT_EQ(database.GetNextId(), 3);
    EXPECT_EQ(database.GetIndex(0).Find(YearKey(2000)), (Positions{0, 1}));
    EXPECT_EQ(database.GetIndex(1).Find(LastNameAttribute().ParseKey(
                  ArraySequence<std::string>{"Петров"})),
              (Positions{1}));
}

TEST(DatabaseTest, RejectedPersonChangesNothing)
{
    Database database;
    database.AddPerson(Person(1, "Иванов", "Пётр", "", 1950));
    database.CreateIndex(CompositeAttribute(Attribute::AgeAt(1980)), SearchMethod::Avl);
    EXPECT_THROW(database.AddPerson(Person(2, "Петров", "Иван", "", 1990)), InvalidArgument);
    EXPECT_EQ(database.GetCount(), 1u);
    EXPECT_EQ(database.GetIndex(0).GetKeyCount(), 1u);
}

TEST(DatabaseTest, ReplacePeopleRebuildsIndexes)
{
    Database database;
    database.CreateIndex(BirthYearAttribute(), SearchMethod::Avl);
    PersonGenerator generator(kSeed);
    database.ReplacePeople(generator.Generate(300, 1980, 1990));
    EXPECT_EQ(database.GetCount(), 300u);
    EXPECT_EQ(database.GetNextId(), 301);
    EXPECT_EQ(database.GetIndex(0).FindRange(YearKey(1980), YearKey(1990)).GetLength(), 300u);
}

TEST(DatabaseTest, FailedReplaceKeepsOldState)
{
    Database database;
    database.AddPerson(Person(1, "Иванов", "Пётр", "", 1950));
    database.CreateIndex(CompositeAttribute(Attribute::AgeAt(1980)), SearchMethod::BinarySearch);

    const ArraySequence<Person> tooYoung{Person(1, "Петров", "Иван", "", 1990)};
    EXPECT_THROW(database.ReplacePeople(tooYoung), InvalidArgument);
    EXPECT_EQ(database.GetCount(), 1u);
    EXPECT_EQ(database.GetPeople().Get(0).GetLastName(), "Иванов");
    EXPECT_EQ(database.GetIndex(0).Find(YearKey(30)), (Positions{0}));
}

TEST(DatabaseTest, RemoveIndex)
{
    Database database;
    database.CreateIndex(BirthYearAttribute(), SearchMethod::Avl);
    database.CreateIndex(LastNameAttribute(), SearchMethod::Linear);
    database.RemoveIndex(0);
    EXPECT_EQ(database.GetIndexCount(), 1u);
    EXPECT_EQ(database.GetIndex(0).GetMethod(), SearchMethod::Linear);
    EXPECT_THROW(database.RemoveIndex(1), IndexOutOfRange);
    EXPECT_THROW(database.GetIndex(1), IndexOutOfRange);
}
