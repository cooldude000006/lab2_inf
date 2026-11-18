// Тесты исключений, пар, правил сравнения и интерфейсов.

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "Compare.h"
#include "Exceptions.h"
#include "IDictionary.h"
#include "IOrderedDictionary.h"
#include "ISortedSequence.h"
#include "Pair.h"
#include "TestHelpers.h"

// --- Исключения -------------------------------------------------------------

TEST(Exceptions, AllDeriveFromLab2Error)
{
    EXPECT_TRUE((std::is_base_of_v<Lab2Error, IndexOutOfRange>));
    EXPECT_TRUE((std::is_base_of_v<Lab2Error, KeyNotFound>));
    EXPECT_TRUE((std::is_base_of_v<Lab2Error, DuplicateKey>));
    EXPECT_TRUE((std::is_base_of_v<Lab2Error, InvalidArgument>));
    EXPECT_TRUE((std::is_base_of_v<std::exception, Lab2Error>));
}

TEST(Exceptions, CaughtAsBaseKeepsMessage)
{
    try
    {
        throw KeyNotFound("ключ «Иванов» не найден");
    }
    catch (const Lab2Error& error)
    {
        EXPECT_EQ(std::string(error.what()), "ключ «Иванов» не найден");
    }
}

// --- Пары и правила сравнения -----------------------------------------------

TEST(PairTest, EqualityComparesKeyAndValue)
{
    Pair<int, std::string> a{1, "x"};
    EXPECT_EQ(a, (Pair<int, std::string>{1, "x"}));
    EXPECT_NE(a, (Pair<int, std::string>{1, "y"}));
    EXPECT_NE(a, (Pair<int, std::string>{2, "x"}));
}

TEST(PairTest, KeyLessIgnoresValue)
{
    PairKeyLess<int, std::string> less;
    EXPECT_TRUE(less({1, "z"}, {2, "a"}));
    EXPECT_FALSE(less({2, "a"}, {1, "z"}));
    // Одинаковые ключи: ни одна пара не меньше другой, то есть они
    // равны с точки зрения порядка, хотя значения разные.
    EXPECT_FALSE(less({1, "a"}, {1, "b"}));
    EXPECT_FALSE(less({1, "b"}, {1, "a"}));
}

TEST(CompareTest, LessUsesOperatorLess)
{
    Less<int> less;
    EXPECT_TRUE(less(1, 2));
    EXPECT_FALSE(less(2, 1));
    EXPECT_FALSE(less(2, 2));

    Less<std::string> stringLess;
    EXPECT_TRUE(stringLess("Иванов", "Петров"));
}

// --- Интерфейсы -------------------------------------------------------------

TEST(Interfaces, AreAbstractWithVirtualDestructor)
{
    EXPECT_TRUE(std::is_abstract_v<ISortedSequence<int>>);
    EXPECT_TRUE((std::is_abstract_v<IDictionary<int, int>>));
    EXPECT_TRUE((std::is_abstract_v<IOrderedDictionary<int, int>>));

    EXPECT_TRUE(std::has_virtual_destructor_v<ISortedSequence<int>>);
    EXPECT_TRUE((std::has_virtual_destructor_v<IDictionary<int, int>>));
    EXPECT_TRUE((std::is_base_of_v<IDictionary<int, int>, IOrderedDictionary<int, int>>));
}
