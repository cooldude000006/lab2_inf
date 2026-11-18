// Тесты отсортированной последовательности и бинарного поиска (DESIGN.md, п. 4.2).

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "Pair.h"
#include "SortedSequence.h"
#include "TestHelpers.h"

namespace
{

// Порядок по убыванию — проверка, что правило сравнения действительно
// берётся из параметра шаблона.
class Greater
{
public:
    bool operator()(int left, int right) const
    {
        return left > right;
    }
};

using Tagged = Pair<int, std::string>;
using TaggedSequence = SortedSequence<Tagged, PairKeyLess<int, std::string>>;

} // namespace

// --- Добавление и порядок ---------------------------------------------------

TEST(SortedSequenceAdd, KeepsAscendingOrder)
{
    SortedSequence<int> sequence(ArraySequence<int>{5, 1, 4, 2, 3});
    EXPECT_EQ(sequence.ToArraySequence(), (ArraySequence<int>{1, 2, 3, 4, 5}));
    EXPECT_EQ(sequence.GetLength(), 5u);
    EXPECT_EQ(sequence.GetFirst(), 1);
    EXPECT_EQ(sequence.GetLast(), 5);
}

TEST(SortedSequenceAdd, EqualElementsKeepInsertionOrder)
{
    TaggedSequence sequence;
    sequence.Add({2, "first"});
    sequence.Add({1, "x"});
    sequence.Add({2, "second"});
    sequence.Add({3, "y"});
    sequence.Add({2, "third"});

    ASSERT_EQ(sequence.GetLength(), 5u);
    EXPECT_EQ(sequence.Get(1).value, "first");
    EXPECT_EQ(sequence.Get(2).value, "second");
    EXPECT_EQ(sequence.Get(3).value, "third");
}

TEST(SortedSequenceAdd, InsertReturnsPosition)
{
    SortedSequence<int> sequence(ArraySequence<int>{10, 30});
    EXPECT_EQ(sequence.Insert(20), 1u);
    EXPECT_EQ(sequence.Insert(5), 0u);
    EXPECT_EQ(sequence.Insert(40), 4u);
    EXPECT_EQ(sequence.Insert(20), 3u); // после равного
}

TEST(SortedSequenceAdd, CustomCompareGivesDescendingOrder)
{
    SortedSequence<int, Greater> sequence(ArraySequence<int>{2, 9, 4});
    EXPECT_EQ(sequence.ToArraySequence(), (ArraySequence<int>{9, 4, 2}));
    EXPECT_EQ(sequence.IndexOf(4), 1);
}

TEST(SortedSequenceAdd, RandomDataMatchesStdSort)
{
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(-50, 50);
    std::vector<int> expected;
    SortedSequence<int> sequence;
    for (int i = 0; i < 500; ++i)
    {
        const int value = distribution(generator);
        expected.push_back(value);
        sequence.Add(value);
    }
    std::sort(expected.begin(), expected.end());

    ASSERT_EQ(sequence.GetLength(), expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(sequence.Get(i), expected[i]);
    }
}

// --- Бинарный поиск границ --------------------------------------------------

TEST(SortedSequenceBounds, PresentElementWithDuplicates)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 3, 3, 3, 5});
    EXPECT_EQ(sequence.LowerBound(3), 1u);
    EXPECT_EQ(sequence.UpperBound(3), 4u);
}

TEST(SortedSequenceBounds, AbsentElementBetween)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 3, 5});
    EXPECT_EQ(sequence.LowerBound(4), 2u);
    EXPECT_EQ(sequence.UpperBound(4), 2u);
}

TEST(SortedSequenceBounds, BelowAndAboveAll)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 3, 5});
    EXPECT_EQ(sequence.LowerBound(0), 0u);
    EXPECT_EQ(sequence.UpperBound(0), 0u);
    EXPECT_EQ(sequence.LowerBound(9), 3u);
    EXPECT_EQ(sequence.UpperBound(9), 3u);
}

TEST(SortedSequenceBounds, EmptySequence)
{
    SortedSequence<int> sequence;
    EXPECT_EQ(sequence.LowerBound(1), 0u);
    EXPECT_EQ(sequence.UpperBound(1), 0u);
}

TEST(SortedSequenceBounds, MatchStdBoundsOnRandomData)
{
    std::mt19937 generator(7);
    std::uniform_int_distribution<int> distribution(0, 30);
    std::vector<int> values;
    SortedSequence<int> sequence;
    for (int i = 0; i < 200; ++i)
    {
        const int value = distribution(generator);
        values.push_back(value);
        sequence.Add(value);
    }
    std::sort(values.begin(), values.end());

    for (int query = -2; query <= 32; ++query)
    {
        const auto lower = std::lower_bound(values.begin(), values.end(), query);
        const auto upper = std::upper_bound(values.begin(), values.end(), query);
        EXPECT_EQ(sequence.LowerBound(query),
                  static_cast<std::size_t>(lower - values.begin()));
        EXPECT_EQ(sequence.UpperBound(query),
                  static_cast<std::size_t>(upper - values.begin()));
    }
}

// --- Поиск элемента ---------------------------------------------------------

TEST(SortedSequenceFind, IndexOfReturnsFirstOfEqual)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 2, 2, 2, 3});
    EXPECT_EQ(sequence.IndexOf(2), 1);
    EXPECT_EQ(sequence.IndexOf(1), 0);
    EXPECT_EQ(sequence.IndexOf(3), 4);
}

TEST(SortedSequenceFind, IndexOfMissingIsNotFound)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 3});
    EXPECT_EQ(sequence.IndexOf(2), ISortedSequence<int>::kNotFound);
    EXPECT_EQ(sequence.IndexOf(0), -1);
    EXPECT_EQ(sequence.IndexOf(4), -1);
    EXPECT_FALSE(sequence.Contains(2));
    EXPECT_TRUE(sequence.Contains(3));

    SortedSequence<int> empty;
    EXPECT_EQ(empty.IndexOf(1), -1);
}

TEST(SortedSequenceFind, StringsInDictionaryOrder)
{
    SortedSequence<std::string> sequence(ArraySequence<std::string>{"Петров", "Иванов", "Сидоров"});
    EXPECT_EQ(sequence.GetFirst(), "Иванов");
    EXPECT_EQ(sequence.IndexOf("Петров"), 1);
}

// --- Поиск по диапазону -----------------------------------------------------

TEST(SortedSequenceRange, InclusiveBounds)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 2, 3, 3, 4, 5, 6});
    EXPECT_EQ(sequence.GetRange(3, 5), (ArraySequence<int>{3, 3, 4, 5}));
}

TEST(SortedSequenceRange, BoundsNotPresent)
{
    SortedSequence<int> sequence(ArraySequence<int>{10, 20, 30, 40});
    EXPECT_EQ(sequence.GetRange(15, 35), (ArraySequence<int>{20, 30}));
    EXPECT_EQ(sequence.GetRange(0, 100), sequence.ToArraySequence());
}

TEST(SortedSequenceRange, SinglePointAndEmptyResult)
{
    SortedSequence<int> sequence(ArraySequence<int>{10, 20, 20, 30});
    EXPECT_EQ(sequence.GetRange(20, 20), (ArraySequence<int>{20, 20}));
    EXPECT_TRUE(sequence.GetRange(21, 29).GetIsEmpty());
    EXPECT_TRUE(sequence.GetRange(31, 50).GetIsEmpty());

    SortedSequence<int> empty;
    EXPECT_TRUE(empty.GetRange(0, 10).GetIsEmpty());
}

TEST(SortedSequenceRange, ReversedBoundsThrow)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 2, 3});
    EXPECT_THROW(sequence.GetRange(3, 1), InvalidArgument);
}

// --- Удаление, доступ, подпоследовательность --------------------------------

TEST(SortedSequenceModify, RemoveAtKeepsOrder)
{
    SortedSequence<int> sequence(ArraySequence<int>{1, 2, 3, 4});
    sequence.RemoveAt(1);
    EXPECT_EQ(sequence.ToArraySequence(), (ArraySequence<int>{1, 3, 4}));
    EXPECT_THROW(sequence.RemoveAt(3), IndexOutOfRange);
}

TEST(SortedSequenceModify, AccessToEmptyThrows)
{
    SortedSequence<int> sequence;
    EXPECT_TRUE(sequence.GetIsEmpty());
    EXPECT_THROW(sequence.Get(0), IndexOutOfRange);
    EXPECT_THROW(sequence.GetFirst(), IndexOutOfRange);
    EXPECT_THROW(sequence.GetLast(), IndexOutOfRange);
}

TEST(SortedSequenceModify, SubsequenceStaysSorted)
{
    SortedSequence<int> sequence(ArraySequence<int>{5, 1, 4, 2, 3});
    SortedSequence<int> part = sequence.GetSubsequence(1, 3);
    EXPECT_EQ(part.ToArraySequence(), (ArraySequence<int>{2, 3, 4}));
    part.Add(0);
    EXPECT_EQ(part.GetFirst(), 0);
    EXPECT_THROW(sequence.GetSubsequence(3, 1), InvalidArgument);
    EXPECT_THROW(sequence.GetSubsequence(0, 5), IndexOutOfRange);
}

TEST(SortedSequenceModify, ClearAndReuse)
{
    SortedSequence<int> sequence(ArraySequence<int>{3, 1});
    sequence.Clear();
    EXPECT_TRUE(sequence.GetIsEmpty());
    sequence.Add(2);
    EXPECT_EQ(sequence.ToArraySequence(), (ArraySequence<int>{2}));
}

TEST(SortedSequenceModify, WorksThroughInterface)
{
    SortedSequence<int> sequence;
    ISortedSequence<int>& base = sequence;
    base.Add(3);
    base.Add(1);
    EXPECT_EQ(base.GetFirst(), 1);
    EXPECT_EQ(base.IndexOf(3), 1);
}
