// Тесты человека, значений атрибутов, составных ключей и атрибутов
// (DESIGN.md, п. 4.5).

#include <gtest/gtest.h>
#include <string>

#include "Attribute.h"
#include "AttributeValue.h"
#include "CompositeKey.h"
#include "Exceptions.h"
#include "Person.h"
#include "TestHelpers.h"

namespace
{

Person MakeIvanov()
{
    return Person(1, "Иванов", "Пётр", "Сергеевич", 1995);
}

CompositeKey Key(std::initializer_list<AttributeValue> values)
{
    return CompositeKey(ArraySequence<AttributeValue>(values));
}

} // namespace

// --- Человек ----------------------------------------------------------------

TEST(PersonTest, FieldsAndDerivedNames)
{
    const Person person = MakeIvanov();
    EXPECT_EQ(person.GetId(), 1);
    EXPECT_EQ(person.GetLastName(), "Иванов");
    EXPECT_EQ(person.GetFirstName(), "Пётр");
    EXPECT_EQ(person.GetMiddleName(), "Сергеевич");
    EXPECT_EQ(person.GetBirthYear(), 1995);
    EXPECT_EQ(person.GetFullName(), "Иванов Пётр Сергеевич");
    EXPECT_EQ(person.GetFio(), "Иванов П.С.");
}

TEST(PersonTest, WithoutMiddleName)
{
    const Person person(2, "Smith", "John", "", 1980);
    EXPECT_EQ(person.GetFullName(), "Smith John");
    EXPECT_EQ(person.GetFio(), "Smith J.");
}

TEST(PersonTest, Age)
{
    const Person person = MakeIvanov();
    EXPECT_EQ(person.GetAge(2019), 24);
    EXPECT_EQ(person.GetAge(1995), 0);
    EXPECT_THROW(person.GetAge(1994), InvalidArgument);
}

TEST(PersonTest, InvalidFieldsThrow)
{
    EXPECT_THROW(Person(0, "Иванов", "Пётр", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, "", "Пётр", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, "   ", "Пётр", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, "Иванов", "", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, " Иванов", "Пётр", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, "Ива;нов", "Пётр", "", 1995), InvalidArgument);
    EXPECT_THROW(Person(1, "Иванов", "Пётр", "", 1899), InvalidArgument);
    EXPECT_THROW(Person(1, "Иванов", "Пётр", "", CurrentYear() + 1), InvalidArgument);
    EXPECT_NO_THROW(Person(1, "Иванов", "Пётр", "", Person::kMinBirthYear));
    EXPECT_NO_THROW(Person(1, "Иванов", "Пётр", "", CurrentYear()));
}

TEST(PersonTest, Equality)
{
    EXPECT_EQ(MakeIvanov(), MakeIvanov());
    EXPECT_NE(MakeIvanov(), Person(1, "Иванов", "Пётр", "Сергеевич", 1996));
}

// --- Значения атрибутов -----------------------------------------------------

TEST(AttributeValueTest, OrderAndEquality)
{
    EXPECT_LT(AttributeValue(1990), AttributeValue(2000));
    EXPECT_LT(AttributeValue(std::string("Иванов")), AttributeValue(std::string("Петров")));
    EXPECT_EQ(AttributeValue(5), AttributeValue(5));
    EXPECT_NE(AttributeValue(5), AttributeValue(std::string("5")));
    // Любое число меньше любой строки — порядок полный.
    EXPECT_LT(AttributeValue(99999), AttributeValue(std::string("")));
    EXPECT_GE(AttributeValue(3), AttributeValue(3));
    EXPECT_LE(AttributeValue(3), AttributeValue(4));
    EXPECT_GT(AttributeValue(4), AttributeValue(3));
}

TEST(AttributeValueTest, AccessByKind)
{
    EXPECT_EQ(AttributeValue(7).GetInteger(), 7);
    EXPECT_EQ(AttributeValue(std::string("x")).GetString(), "x");
    EXPECT_THROW(AttributeValue(7).GetString(), InvalidArgument);
    EXPECT_THROW(AttributeValue(std::string("x")).GetInteger(), InvalidArgument);
    EXPECT_EQ(AttributeValue(7).ToString(), "7");
}

// --- Составной ключ ---------------------------------------------------------

TEST(CompositeKeyTest, LexicographicOrder)
{
    const AttributeValue ivanov(std::string("Иванов"));
    const AttributeValue petrov(std::string("Петров"));
    EXPECT_LT(Key({ivanov, AttributeValue(2000)}), Key({petrov, AttributeValue(1900)}));
    EXPECT_LT(Key({ivanov, AttributeValue(1990)}), Key({ivanov, AttributeValue(2000)}));
    EXPECT_EQ(Key({ivanov, AttributeValue(1990)}), Key({ivanov, AttributeValue(1990)}));
    // Начало ключа меньше самого ключа.
    EXPECT_LT(Key({ivanov}), Key({ivanov, AttributeValue(1900)}));
}

TEST(CompositeKeyTest, ToString)
{
    EXPECT_EQ(Key({AttributeValue(std::string("Иванов"))}).ToString(), "Иванов");
    EXPECT_EQ(Key({AttributeValue(std::string("Иванов")), AttributeValue(1995)}).ToString(),
              "(Иванов, 1995)");
}

// --- Атрибуты ---------------------------------------------------------------

TEST(AttributeTest, ExtractAllAttributes)
{
    const Person person = MakeIvanov();
    EXPECT_EQ(Attribute(AttributeId::Id).ExtractFrom(person), AttributeValue(1));
    EXPECT_EQ(Attribute(AttributeId::LastName).ExtractFrom(person).GetString(), "Иванов");
    EXPECT_EQ(Attribute(AttributeId::FirstName).ExtractFrom(person).GetString(), "Пётр");
    EXPECT_EQ(Attribute(AttributeId::MiddleName).ExtractFrom(person).GetString(), "Сергеевич");
    EXPECT_EQ(Attribute(AttributeId::FullName).ExtractFrom(person).GetString(),
              "Иванов Пётр Сергеевич");
    EXPECT_EQ(Attribute(AttributeId::Fio).ExtractFrom(person).GetString(), "Иванов П.С.");
    EXPECT_EQ(Attribute(AttributeId::BirthYear).ExtractFrom(person), AttributeValue(1995));
    EXPECT_EQ(Attribute::AgeAt(2019).ExtractFrom(person), AttributeValue(24));
}

TEST(AttributeTest, AgeNeedsYear)
{
    EXPECT_THROW(Attribute(AttributeId::Age), InvalidArgument);
    EXPECT_THROW(Attribute::AgeAt(1800), InvalidArgument);
    EXPECT_EQ(Attribute::AgeAt(2019).GetName(), "возраст на 2019 год");
    EXPECT_THROW(Attribute::AgeAt(1990).ExtractFrom(MakeIvanov()), InvalidArgument);
}

TEST(AttributeTest, KindsAndNames)
{
    EXPECT_EQ(Attribute(AttributeId::BirthYear).GetKind(), ValueKind::Integer);
    EXPECT_EQ(Attribute(AttributeId::LastName).GetKind(), ValueKind::String);
    EXPECT_EQ(Attribute(AttributeId::LastName).GetName(), "фамилия");
    EXPECT_EQ(Attribute::GetPlainAttributes().GetLength(), 7u);
    EXPECT_NE(Attribute::AgeAt(2019), Attribute::AgeAt(2020));
}

TEST(AttributeTest, Parse)
{
    EXPECT_EQ(Attribute(AttributeId::BirthYear).Parse(" 1995 "), AttributeValue(1995));
    EXPECT_EQ(Attribute(AttributeId::LastName).Parse(" Иванов "),
              AttributeValue(std::string("Иванов")));
    EXPECT_THROW(Attribute(AttributeId::BirthYear).Parse("девяносто"), InvalidArgument);
    EXPECT_THROW(Attribute(AttributeId::LastName).Parse("  "), InvalidArgument);
}

// --- Составной атрибут ------------------------------------------------------

TEST(CompositeAttributeTest, MakeKeyAndName)
{
    const CompositeAttribute attribute(ArraySequence<Attribute>{
        Attribute(AttributeId::LastName), Attribute(AttributeId::BirthYear)});
    EXPECT_EQ(attribute.GetSize(), 2u);
    EXPECT_EQ(attribute.GetName(), "фамилия + год рождения");
    EXPECT_EQ(attribute.MakeKey(MakeIvanov()),
              Key({AttributeValue(std::string("Иванов")), AttributeValue(1995)}));
}

TEST(CompositeAttributeTest, InvalidSetsThrow)
{
    EXPECT_THROW(CompositeAttribute(ArraySequence<Attribute>()), InvalidArgument);
    EXPECT_THROW(CompositeAttribute(ArraySequence<Attribute>{Attribute(AttributeId::LastName),
                                                             Attribute(AttributeId::LastName)}),
                 InvalidArgument);
    // Возраст на разные годы — разные атрибуты.
    EXPECT_NO_THROW(CompositeAttribute(
        ArraySequence<Attribute>{Attribute::AgeAt(2019), Attribute::AgeAt(2020)}));
}

TEST(CompositeAttributeTest, ParseKey)
{
    const CompositeAttribute attribute(ArraySequence<Attribute>{
        Attribute(AttributeId::LastName), Attribute(AttributeId::BirthYear)});
    EXPECT_EQ(attribute.ParseKey(ArraySequence<std::string>{"Иванов", "1995"}),
              attribute.MakeKey(MakeIvanov()));
    EXPECT_THROW(attribute.ParseKey(ArraySequence<std::string>{"Иванов"}), InvalidArgument);
    EXPECT_THROW(attribute.ParseKey(ArraySequence<std::string>{"Иванов", "год"}),
                 InvalidArgument);
}

TEST(CompositeAttributeTest, CheckKey)
{
    const CompositeAttribute attribute(Attribute(AttributeId::BirthYear));
    EXPECT_NO_THROW(attribute.CheckKey(Key({AttributeValue(1995)})));
    EXPECT_THROW(attribute.CheckKey(Key({AttributeValue(std::string("1995"))})),
                 InvalidArgument);
    EXPECT_THROW(attribute.CheckKey(Key({AttributeValue(1995), AttributeValue(1)})),
                 InvalidArgument);
    EXPECT_THROW(attribute.CheckKey(CompositeKey()), InvalidArgument);
}
