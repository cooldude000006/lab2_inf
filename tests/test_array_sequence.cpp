// Тесты ArraySequence (DESIGN.md, п. 4.1).

#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "ArraySequence.h"
#include "Exceptions.h"
#include "TestHelpers.h"

namespace
{

// Последовательность 0, 1, ..., count - 1.
ArraySequence<int> MakeRange(int count)
{
    ArraySequence<int> sequence;
    for (int i = 0; i < count; ++i)
    {
        sequence.Append(i);
    }
    return sequence;
}

} // namespace

// --- Создание ---------------------------------------------------------------

TEST(ArraySequenceCreate, DefaultIsEmpty)
{
    ArraySequence<int> sequence;
    EXPECT_EQ(sequence.GetLength(), 0u);
    EXPECT_TRUE(sequence.GetIsEmpty());
    EXPECT_EQ(sequence.GetCapacity(), 0u);
}

TEST(ArraySequenceCreate, FromArrayCopiesElements)
{
    const int items[] = {5, 3, 8};
    ArraySequence<int> sequence(items, 3);
    EXPECT_EQ(sequence, (ArraySequence<int>{5, 3, 8}));
    EXPECT_FALSE(sequence.GetIsEmpty());
}

TEST(ArraySequenceCreate, FromNullArrayWithZeroCountIsEmpty)
{
    ArraySequence<int> sequence(nullptr, 0);
    EXPECT_TRUE(sequence.GetIsEmpty());
}

TEST(ArraySequenceCreate, FromNullArrayWithPositiveCountThrows)
{
    EXPECT_THROW(ArraySequence<int>(nullptr, 3), InvalidArgument);
}

TEST(ArraySequenceCreate, FromInitializerList)
{
    ArraySequence<std::string> sequence{"a", "b"};
    EXPECT_EQ(sequence.GetLength(), 2u);
    EXPECT_EQ(sequence.Get(0), "a");
    EXPECT_EQ(sequence.Get(1), "b");
}

// --- Чтение -----------------------------------------------------------------

TEST(ArraySequenceRead, GetFirstLast)
{
    ArraySequence<int> sequence{4, 5, 6};
    EXPECT_EQ(sequence.Get(1), 5);
    EXPECT_EQ(sequence.GetFirst(), 4);
    EXPECT_EQ(sequence.GetLast(), 6);
}

TEST(ArraySequenceRead, GetOutOfRangeThrows)
{
    ArraySequence<int> sequence{1, 2, 3};
    EXPECT_THROW(sequence.Get(3), IndexOutOfRange);
    EXPECT_THROW(sequence.Get(100), IndexOutOfRange);
}

TEST(ArraySequenceRead, EmptySequenceAccessThrows)
{
    ArraySequence<int> sequence;
    EXPECT_THROW(sequence.Get(0), IndexOutOfRange);
    EXPECT_THROW(sequence.GetFirst(), IndexOutOfRange);
    EXPECT_THROW(sequence.GetLast(), IndexOutOfRange);
}

TEST(ArraySequenceRead, SubsequenceInclusiveBounds)
{
    ArraySequence<int> sequence = MakeRange(6);
    EXPECT_EQ(sequence.GetSubsequence(1, 3), (ArraySequence<int>{1, 2, 3}));
    EXPECT_EQ(sequence.GetSubsequence(0, 5), sequence);
    EXPECT_EQ(sequence.GetSubsequence(4, 4), (ArraySequence<int>{4}));
}

TEST(ArraySequenceRead, SubsequenceInvalidBoundsThrow)
{
    ArraySequence<int> sequence = MakeRange(6);
    EXPECT_THROW(sequence.GetSubsequence(3, 1), InvalidArgument);
    EXPECT_THROW(sequence.GetSubsequence(2, 6), IndexOutOfRange);

    ArraySequence<int> empty;
    EXPECT_THROW(empty.GetSubsequence(0, 0), IndexOutOfRange);
}

// --- Изменение --------------------------------------------------------------

TEST(ArraySequenceModify, AppendManyElementsGrowsCapacity)
{
    const int count = 1000;
    ArraySequence<int> sequence = MakeRange(count);
    ASSERT_EQ(sequence.GetLength(), static_cast<std::size_t>(count));
    EXPECT_GE(sequence.GetCapacity(), sequence.GetLength());
    for (int i = 0; i < count; ++i)
    {
        EXPECT_EQ(sequence.Get(static_cast<std::size_t>(i)), i);
    }
}

TEST(ArraySequenceModify, PrependReversesOrder)
{
    ArraySequence<int> sequence;
    sequence.Prepend(1);
    sequence.Prepend(2);
    sequence.Prepend(3);
    EXPECT_EQ(sequence, (ArraySequence<int>{3, 2, 1}));
}

TEST(ArraySequenceModify, InsertAtBeginningMiddleEnd)
{
    ArraySequence<int> sequence{1, 3};
    sequence.InsertAt(2, 1);
    sequence.InsertAt(0, 0);
    sequence.InsertAt(4, 4);
    EXPECT_EQ(sequence, (ArraySequence<int>{0, 1, 2, 3, 4}));
}

TEST(ArraySequenceModify, InsertAtBeyondLengthThrows)
{
    ArraySequence<int> sequence{1, 2};
    EXPECT_THROW(sequence.InsertAt(9, 3), IndexOutOfRange);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 2}));
}

TEST(ArraySequenceModify, AppendOwnElementWhenFull)
{
    // Ёмкость заполнена, значит Append перевыделит память; ссылка на
    // собственный элемент не должна стать висячей.
    ArraySequence<std::string> sequence;
    sequence.Append("first");
    while (sequence.GetLength() < sequence.GetCapacity())
    {
        sequence.Append("x");
    }
    sequence.Append(sequence.Get(0));
    EXPECT_EQ(sequence.GetLast(), "first");
}

TEST(ArraySequenceModify, InsertOwnElementIntoMiddle)
{
    ArraySequence<std::string> sequence{"a", "b", "c"};
    sequence.Reserve(10);
    sequence.InsertAt(sequence.Get(1), 0);
    EXPECT_EQ(sequence, (ArraySequence<std::string>{"b", "a", "b", "c"}));
}

TEST(ArraySequenceModify, RemoveAtFirstMiddleLast)
{
    ArraySequence<int> sequence = MakeRange(5);
    sequence.RemoveAt(0);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 2, 3, 4}));
    sequence.RemoveAt(1);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 3, 4}));
    sequence.RemoveAt(2);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 3}));
}

TEST(ArraySequenceModify, RemoveAtInvalidIndexThrows)
{
    ArraySequence<int> sequence{1};
    EXPECT_THROW(sequence.RemoveAt(1), IndexOutOfRange);
    sequence.RemoveAt(0);
    EXPECT_THROW(sequence.RemoveAt(0), IndexOutOfRange);
}

TEST(ArraySequenceModify, SetReplacesElement)
{
    ArraySequence<int> sequence{1, 2, 3};
    sequence.Set(1, 20);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 20, 3}));
    EXPECT_THROW(sequence.Set(3, 0), IndexOutOfRange);
}

TEST(ArraySequenceModify, MutableGetChangesElementInPlace)
{
    ArraySequence<ArraySequence<int>> lists{ArraySequence<int>{1}, ArraySequence<int>{2}};
    lists.Get(1).Append(3);
    EXPECT_EQ(lists.Get(1), (ArraySequence<int>{2, 3}));
    EXPECT_THROW(lists.Get(2), IndexOutOfRange);
}

TEST(ArraySequenceModify, ClearKeepsCapacity)
{
    ArraySequence<int> sequence = MakeRange(10);
    const std::size_t capacity = sequence.GetCapacity();
    sequence.Clear();
    EXPECT_TRUE(sequence.GetIsEmpty());
    EXPECT_EQ(sequence.GetCapacity(), capacity);
    sequence.Append(7);
    EXPECT_EQ(sequence, (ArraySequence<int>{7}));
}

TEST(ArraySequenceModify, ReserveDoesNotChangeLength)
{
    ArraySequence<int> sequence{1, 2};
    sequence.Reserve(100);
    EXPECT_GE(sequence.GetCapacity(), 100u);
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 2}));

    sequence.Reserve(1);
    EXPECT_GE(sequence.GetCapacity(), 100u);
}

// --- Копирование и перемещение ---------------------------------------------

TEST(ArraySequenceCopy, CopyIsIndependent)
{
    ArraySequence<int> original{1, 2, 3};
    ArraySequence<int> copy(original);
    copy.Set(0, 100);
    copy.Append(4);
    EXPECT_EQ(original, (ArraySequence<int>{1, 2, 3}));
    EXPECT_EQ(copy, (ArraySequence<int>{100, 2, 3, 4}));
}

TEST(ArraySequenceCopy, CopyAssignment)
{
    ArraySequence<int> source{1, 2};
    ArraySequence<int> target{9, 9, 9};
    target = source;
    EXPECT_EQ(target, source);
    source.Append(3);
    EXPECT_EQ(target, (ArraySequence<int>{1, 2}));
}

TEST(ArraySequenceCopy, SelfAssignment)
{
    ArraySequence<int> sequence{1, 2, 3};
    ArraySequence<int>& alias = sequence;
    sequence = alias;
    EXPECT_EQ(sequence, (ArraySequence<int>{1, 2, 3}));
}

TEST(ArraySequenceCopy, MoveLeavesSourceEmpty)
{
    ArraySequence<int> source{1, 2, 3};
    ArraySequence<int> target(std::move(source));
    EXPECT_EQ(target, (ArraySequence<int>{1, 2, 3}));
    EXPECT_TRUE(source.GetIsEmpty()); // NOLINT(bugprone-use-after-move): проверяем состояние
}

TEST(ArraySequenceCopy, MoveAssignment)
{
    ArraySequence<int> source{1, 2};
    ArraySequence<int> target{7};
    target = std::move(source);
    EXPECT_EQ(target, (ArraySequence<int>{1, 2}));
    EXPECT_TRUE(source.GetIsEmpty()); // NOLINT(bugprone-use-after-move): проверяем состояние
}

TEST(ArraySequenceCopy, EmptyCopy)
{
    ArraySequence<int> empty;
    ArraySequence<int> copy(empty);
    EXPECT_TRUE(copy.GetIsEmpty());
}

// --- Сравнение --------------------------------------------------------------

TEST(ArraySequenceCompare, Equality)
{
    EXPECT_EQ((ArraySequence<int>{1, 2}), (ArraySequence<int>{1, 2}));
    EXPECT_NE((ArraySequence<int>{1, 2}), (ArraySequence<int>{2, 1}));
    EXPECT_NE((ArraySequence<int>{1, 2}), (ArraySequence<int>{1, 2, 3}));
    EXPECT_EQ(ArraySequence<int>(), ArraySequence<int>());
}
