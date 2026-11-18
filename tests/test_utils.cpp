// Тесты текстовых функций и сортировки слиянием.

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "MergeSort.h"
#include "Pair.h"
#include "TestHelpers.h"
#include "TextUtils.h"

// --- Текст ------------------------------------------------------------------

TEST(TextUtils, Trim)
{
    EXPECT_EQ(Trim("  Иванов \t\r\n"), "Иванов");
    EXPECT_EQ(Trim("a b"), "a b");
    EXPECT_EQ(Trim("   "), "");
    EXPECT_EQ(Trim(""), "");
}

TEST(TextUtils, ParseIntegerValid)
{
    EXPECT_EQ(ParseInteger("1995", "год"), 1995);
    EXPECT_EQ(ParseInteger("  -7 ", "число"), -7);
    EXPECT_EQ(ParseInteger("0", "число"), 0);
}

TEST(TextUtils, ParseIntegerInvalid)
{
    EXPECT_THROW(ParseInteger("", "год"), InvalidArgument);
    EXPECT_THROW(ParseInteger("19x5", "год"), InvalidArgument);
    EXPECT_THROW(ParseInteger("12.5", "год"), InvalidArgument);
    EXPECT_THROW(ParseInteger("abc", "год"), InvalidArgument);
    EXPECT_THROW(ParseInteger("99999999999", "год"), InvalidArgument);
}

TEST(TextUtils, ParseIntegerMessageNamesField)
{
    try
    {
        ParseInteger("abc", "год рождения");
        FAIL() << "ожидалось исключение";
    }
    catch (const InvalidArgument& error)
    {
        EXPECT_NE(std::string(error.what()).find("год рождения"), std::string::npos);
    }
}

TEST(TextUtils, FirstUtf8Character)
{
    EXPECT_EQ(FirstUtf8Character("Пётр"), "П");
    EXPECT_EQ(FirstUtf8Character("Ivan"), "I");
    EXPECT_EQ(FirstUtf8Character(""), "");
}

TEST(TextUtils, Utf8LengthAndPadding)
{
    EXPECT_EQ(Utf8Length("размер"), 6u);
    EXPECT_EQ(Utf8Length("size"), 4u);
    EXPECT_EQ(PadRight("АВЛ", 5), "АВЛ  ");
    EXPECT_EQ(PadLeft("12", 4), "  12");
    EXPECT_EQ(PadRight("длинная строка", 3), "длинная строка");
    EXPECT_EQ(FormatFixed(1.23456, 2), "1.23");
}

// --- Сортировка слиянием ----------------------------------------------------

TEST(MergeSortTest, EmptyAndSingle)
{
    ArraySequence<int> empty;
    MergeSort(empty);
    EXPECT_TRUE(empty.GetIsEmpty());

    ArraySequence<int> single{5};
    MergeSort(single);
    EXPECT_EQ(single, (ArraySequence<int>{5}));
}

TEST(MergeSortTest, SortedReversedAndRandomMatchStdSort)
{
    std::mt19937 generator(5);
    std::uniform_int_distribution<int> distribution(-100, 100);
    std::vector<int> random;
    for (int i = 0; i < 1000; ++i)
    {
        random.push_back(distribution(generator));
    }
    std::vector<int> ascending = random;
    std::sort(ascending.begin(), ascending.end());
    std::vector<int> descending(ascending.rbegin(), ascending.rend());

    for (const std::vector<int>& input : {random, ascending, descending})
    {
        ArraySequence<int> items(input.data(), input.size());
        MergeSort(items);
        ASSERT_EQ(items.GetLength(), ascending.size());
        for (std::size_t i = 0; i < ascending.size(); ++i)
        {
            EXPECT_EQ(items.Get(i), ascending[i]);
        }
    }
}

TEST(MergeSortTest, IsStable)
{
    ArraySequence<Pair<int, std::string>> items{{2, "a"}, {1, "b"}, {2, "c"}, {1, "d"}, {2, "e"}};
    MergeSort(items, PairKeyLess<int, std::string>());
    EXPECT_EQ(items, (ArraySequence<Pair<int, std::string>>{
                         {1, "b"}, {1, "d"}, {2, "a"}, {2, "c"}, {2, "e"}}));
}
