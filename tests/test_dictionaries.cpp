// Тесты словарей (DESIGN.md, п. 4.4).
//
// Один набор проверок запускается для обеих реализаций IOrderedDictionary:
// на отсортированной последовательности и на АВЛ-дереве. Так проверяется
// и каждая реализация, и то, что они ведут себя одинаково.

#include <gtest/gtest.h>
#include <map>
#include <random>
#include <string>

#include "Exceptions.h"
#include "SortedSequenceDictionary.h"
#include "TestHelpers.h"
#include "TreeDictionary.h"

namespace
{

// Обёртка, чтобы GoogleTest мог перебрать шаблоны словарей.
template <template <class, class> class DictionaryTemplate>
class DictionaryKind
{
public:
    template <class K, class V>
    using Type = DictionaryTemplate<K, V>;
};

using DictionaryKinds =
    ::testing::Types<DictionaryKind<SortedSequenceDictionary>, DictionaryKind<TreeDictionary>>;

template <class Kind>
class DictionaryTest : public ::testing::Test
{
protected:
    using Dictionary = typename Kind::template Type<int, std::string>;
    using Entry = Pair<int, std::string>;

    static Dictionary MakeDictionary()
    {
        Dictionary dictionary;
        dictionary.Add(30, "c");
        dictionary.Add(10, "a");
        dictionary.Add(20, "b");
        return dictionary;
    }
};

} // namespace

TYPED_TEST_SUITE(DictionaryTest, DictionaryKinds);

TYPED_TEST(DictionaryTest, AddGetContainsCount)
{
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_EQ(dictionary.GetCount(), 3u);
    EXPECT_GE(dictionary.GetCapacity(), dictionary.GetCount());
    EXPECT_EQ(dictionary.Get(10), "a");
    EXPECT_EQ(dictionary.Get(30), "c");
    EXPECT_TRUE(dictionary.ContainsKey(20));
    EXPECT_FALSE(dictionary.ContainsKey(25));
}

TYPED_TEST(DictionaryTest, EmptyDictionary)
{
    typename TestFixture::Dictionary dictionary;
    EXPECT_EQ(dictionary.GetCount(), 0u);
    EXPECT_FALSE(dictionary.ContainsKey(1));
    EXPECT_THROW(dictionary.Get(1), KeyNotFound);
    EXPECT_THROW(dictionary.Remove(1), KeyNotFound);
    EXPECT_TRUE(dictionary.GetKeys().GetIsEmpty());
    EXPECT_TRUE(dictionary.GetRange(0, 100).GetIsEmpty());
}

TYPED_TEST(DictionaryTest, DuplicateKeyThrowsAndKeepsValue)
{
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_THROW(dictionary.Add(20, "new"), DuplicateKey);
    EXPECT_EQ(dictionary.Get(20), "b");
    EXPECT_EQ(dictionary.GetCount(), 3u);
}

TYPED_TEST(DictionaryTest, MissingKeyThrows)
{
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_THROW(dictionary.Get(15), KeyNotFound);
    EXPECT_THROW(dictionary.Remove(15), KeyNotFound);
    const auto& constant = dictionary;
    EXPECT_THROW(constant.Get(15), KeyNotFound);
    EXPECT_EQ(dictionary.GetCount(), 3u);
}

TYPED_TEST(DictionaryTest, MutableGetChangesValueInPlace)
{
    typename TypeParam::template Type<std::string, ArraySequence<int>> positions;
    positions.Add("Иванов", ArraySequence<int>{0});
    positions.Add("Петров", ArraySequence<int>{1});
    positions.Get("Иванов").Append(5);
    EXPECT_EQ(positions.Get("Иванов"), (ArraySequence<int>{0, 5}));
    EXPECT_EQ(positions.Get("Петров"), (ArraySequence<int>{1}));
}

TYPED_TEST(DictionaryTest, RemoveAndReAdd)
{
    auto dictionary = TestFixture::MakeDictionary();
    dictionary.Remove(20);
    EXPECT_EQ(dictionary.GetCount(), 2u);
    EXPECT_FALSE(dictionary.ContainsKey(20));
    EXPECT_EQ(dictionary.Get(10), "a");
    EXPECT_EQ(dictionary.Get(30), "c");

    dictionary.Add(20, "b2");
    EXPECT_EQ(dictionary.Get(20), "b2");
    EXPECT_EQ(dictionary.Get(30), "c");
}

TYPED_TEST(DictionaryTest, KeysAscending)
{
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_EQ(dictionary.GetKeys(), (ArraySequence<int>{10, 20, 30}));
}

TYPED_TEST(DictionaryTest, RangeInclusive)
{
    using Entry = typename TestFixture::Entry;
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_EQ(dictionary.GetRange(10, 20), (ArraySequence<Entry>{{10, "a"}, {20, "b"}}));
    EXPECT_EQ(dictionary.GetRange(15, 35), (ArraySequence<Entry>{{20, "b"}, {30, "c"}}));
    EXPECT_EQ(dictionary.GetRange(20, 20), (ArraySequence<Entry>{{20, "b"}}));
    EXPECT_TRUE(dictionary.GetRange(11, 19).GetIsEmpty());
}

TYPED_TEST(DictionaryTest, ReversedRangeThrows)
{
    auto dictionary = TestFixture::MakeDictionary();
    EXPECT_THROW(dictionary.GetRange(30, 10), InvalidArgument);
}

TYPED_TEST(DictionaryTest, WorksThroughInterface)
{
    typename TestFixture::Dictionary dictionary;
    IOrderedDictionary<int, std::string>& base = dictionary;
    base.Add(2, "two");
    base.Add(1, "one");
    EXPECT_EQ(base.GetKeys(), (ArraySequence<int>{1, 2}));
    EXPECT_EQ(base.Get(2), "two");
}

TYPED_TEST(DictionaryTest, RandomOperationsMatchStdMap)
{
    std::mt19937 generator(2024);
    std::uniform_int_distribution<int> keyDistribution(0, 99);
    std::uniform_int_distribution<int> actionDistribution(0, 2);

    typename TypeParam::template Type<int, int> dictionary;
    std::map<int, int> expected;
    for (int step = 0; step < 2000; ++step)
    {
        const int key = keyDistribution(generator);
        const int action = actionDistribution(generator);
        if (action == 0 && expected.count(key) == 0)
        {
            dictionary.Add(key, step);
            expected[key] = step;
        }
        else if (action == 1 && expected.count(key) == 1)
        {
            dictionary.Remove(key);
            expected.erase(key);
        }
        ASSERT_EQ(dictionary.ContainsKey(key), expected.count(key) == 1);
    }

    ASSERT_EQ(dictionary.GetCount(), expected.size());
    const ArraySequence<int> keys = dictionary.GetKeys();
    std::size_t i = 0;
    for (const auto& [key, value] : expected)
    {
        EXPECT_EQ(keys.Get(i), key);
        EXPECT_EQ(dictionary.Get(key), value);
        ++i;
    }

    // Диапазоны сверяются с std::map.
    for (int low = -5; low < 100; low += 7)
    {
        const int high = low + 13;
        const auto result = dictionary.GetRange(low, high);
        std::size_t expectedCount = 0;
        for (auto it = expected.lower_bound(low); it != expected.end() && it->first <= high; ++it)
        {
            ASSERT_LT(expectedCount, result.GetLength());
            EXPECT_EQ(result.Get(expectedCount).key, it->first);
            EXPECT_EQ(result.Get(expectedCount).value, it->second);
            ++expectedCount;
        }
        EXPECT_EQ(result.GetLength(), expectedCount);
    }
}

// --- Особенности дерева -----------------------------------------------------

TEST(TreeDictionarySpecific, HeightStaysLogarithmic)
{
    TreeDictionary<int, int> dictionary;
    const int count = 4096;
    for (int i = 0; i < count; ++i)
    {
        dictionary.Add(i, i);
    }
    // Для 4096 упорядоченных ключей высота АВЛ-дерева не больше 1,44·log2(4098) ≈ 17,3.
    EXPECT_LE(dictionary.GetHeight(), 17);
    EXPECT_EQ(dictionary.GetCapacity(), dictionary.GetCount());
}
