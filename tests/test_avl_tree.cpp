// Тесты АВЛ-дерева и построителя (DESIGN.md, п. 4.3).

#include <cmath>
#include <gtest/gtest.h>
#include <random>
#include <set>
#include <string>
#include <utility>

#include "AvlTree.h"
#include "Exceptions.h"
#include "TestHelpers.h"
#include "TreeBuilder.h"

namespace
{

class Greater
{
public:
    bool operator()(int left, int right) const
    {
        return left > right;
    }
};

AvlTree<int> MakeTree(std::initializer_list<int> values)
{
    AvlTree<int> tree;
    for (int value : values)
    {
        tree.Insert(value);
    }
    return tree;
}

// Верхняя оценка высоты АВЛ-дерева из n узлов: 1,44·log2(n + 2).
double MaxAvlHeight(std::size_t count)
{
    const double factor = 1.4405;
    return factor * std::log2(static_cast<double>(count) + 2.0);
}

} // namespace

// --- Вставка и балансировка -------------------------------------------------

TEST(AvlTreeInsert, EmptyTree)
{
    AvlTree<int> tree;
    EXPECT_TRUE(tree.GetIsEmpty());
    EXPECT_EQ(tree.GetCount(), 0u);
    EXPECT_EQ(tree.GetHeight(), 0);
    EXPECT_TRUE(tree.CheckInvariants());
    EXPECT_TRUE(tree.ToArraySequence().GetIsEmpty());
}

// Четыре случая перекоса. Без поворотов каждое дерево было бы цепочкой
// высоты 3, с поворотами высота 2.
TEST(AvlTreeInsert, SmallRightRotation)
{
    AvlTree<int> tree = MakeTree({3, 2, 1});
    EXPECT_EQ(tree.GetHeight(), 2);
    EXPECT_TRUE(tree.CheckInvariants());
}

TEST(AvlTreeInsert, SmallLeftRotation)
{
    AvlTree<int> tree = MakeTree({1, 2, 3});
    EXPECT_EQ(tree.GetHeight(), 2);
    EXPECT_TRUE(tree.CheckInvariants());
}

TEST(AvlTreeInsert, BigRightRotation)
{
    AvlTree<int> tree = MakeTree({3, 1, 2});
    EXPECT_EQ(tree.GetHeight(), 2);
    EXPECT_TRUE(tree.CheckInvariants());
}

TEST(AvlTreeInsert, BigLeftRotation)
{
    AvlTree<int> tree = MakeTree({1, 3, 2});
    EXPECT_EQ(tree.GetHeight(), 2);
    EXPECT_TRUE(tree.CheckInvariants());
}

TEST(AvlTreeInsert, DuplicateIsIgnored)
{
    AvlTree<int> tree = MakeTree({5, 3, 8});
    EXPECT_FALSE(tree.Insert(3));
    EXPECT_TRUE(tree.Insert(4));
    EXPECT_EQ(tree.GetCount(), 4u);
    EXPECT_EQ(tree.ToArraySequence(), (ArraySequence<int>{3, 4, 5, 8}));
}

TEST(AvlTreeInsert, SortedInputStaysBalanced)
{
    const int count = 10000;
    AvlTree<int> ascending;
    AvlTree<int> descending;
    for (int i = 0; i < count; ++i)
    {
        ascending.Insert(i);
        descending.Insert(count - i);
    }
    EXPECT_TRUE(ascending.CheckInvariants());
    EXPECT_TRUE(descending.CheckInvariants());
    EXPECT_LE(ascending.GetHeight(), MaxAvlHeight(count));
    EXPECT_LE(descending.GetHeight(), MaxAvlHeight(count));
}

// --- Удаление ---------------------------------------------------------------

TEST(AvlTreeRemove, LeafNodeWithOneChildAndWithTwoChildren)
{
    AvlTree<int> tree = MakeTree({50, 30, 70, 20, 40, 60, 80, 10});

    tree.Remove(10); // лист
    EXPECT_TRUE(tree.CheckInvariants());
    tree.Remove(20); // теперь лист после удаления 10
    tree.Insert(65);
    tree.Remove(60); // один ребёнок (65)
    EXPECT_TRUE(tree.CheckInvariants());
    tree.Remove(50); // два ребёнка (корень)
    EXPECT_TRUE(tree.CheckInvariants());

    EXPECT_EQ(tree.ToArraySequence(), (ArraySequence<int>{30, 40, 65, 70, 80}));
    EXPECT_EQ(tree.GetCount(), 5u);
}

TEST(AvlTreeRemove, SingleNodeAndReinsert)
{
    AvlTree<int> tree = MakeTree({1});
    tree.Remove(1);
    EXPECT_TRUE(tree.GetIsEmpty());
    EXPECT_EQ(tree.GetHeight(), 0);
    tree.Insert(2);
    EXPECT_EQ(tree.ToArraySequence(), (ArraySequence<int>{2}));
}

TEST(AvlTreeRemove, MissingThrowsAndKeepsTree)
{
    AvlTree<int> tree = MakeTree({1, 2, 3});
    EXPECT_THROW(tree.Remove(5), KeyNotFound);
    EXPECT_EQ(tree.GetCount(), 3u);
    EXPECT_TRUE(tree.CheckInvariants());

    AvlTree<int> empty;
    EXPECT_THROW(empty.Remove(1), KeyNotFound);
}

TEST(AvlTreeRemove, RebalancesAfterRemoval)
{
    // Удаление единственного левого ребёнка даёт перекос «правый-правый»
    // и малый левый поворот: цепочка 20-30-40 высоты 3 становится высоты 2.
    AvlTree<int> tree = MakeTree({20, 10, 30, 40});
    tree.Remove(10);
    EXPECT_EQ(tree.GetHeight(), 2);
    EXPECT_TRUE(tree.CheckInvariants());
}

// --- Поиск ------------------------------------------------------------------

TEST(AvlTreeFind, FindAndContains)
{
    AvlTree<int> tree = MakeTree({5, 2, 8});
    ASSERT_NE(tree.Find(8), nullptr);
    EXPECT_EQ(*tree.Find(8), 8);
    EXPECT_EQ(tree.Find(7), nullptr);
    EXPECT_TRUE(tree.Contains(2));
    EXPECT_FALSE(tree.Contains(3));
}

TEST(AvlTreeFind, FindForUpdateChangesValueNotKey)
{
    AvlTree<Pair<int, std::string>, PairKeyLess<int, std::string>> tree;
    tree.Insert({1, "a"});
    tree.Insert({2, "b"});
    Pair<int, std::string>* entry = tree.FindForUpdate({2, ""});
    ASSERT_NE(entry, nullptr);
    entry->value = "changed";
    EXPECT_EQ(tree.Find({2, ""})->value, "changed");
    EXPECT_EQ(tree.FindForUpdate({3, ""}), nullptr);
}

TEST(AvlTreeFind, RangeInclusiveAndBoundsNotPresent)
{
    AvlTree<int> tree = MakeTree({10, 20, 30, 40, 50, 60});
    EXPECT_EQ(tree.GetRange(20, 40), (ArraySequence<int>{20, 30, 40}));
    EXPECT_EQ(tree.GetRange(15, 45), (ArraySequence<int>{20, 30, 40}));
    EXPECT_EQ(tree.GetRange(0, 100), tree.ToArraySequence());
    EXPECT_EQ(tree.GetRange(30, 30), (ArraySequence<int>{30}));
}

TEST(AvlTreeFind, RangeEmptyAndReversed)
{
    AvlTree<int> tree = MakeTree({10, 20, 30});
    EXPECT_TRUE(tree.GetRange(11, 19).GetIsEmpty());
    EXPECT_TRUE(tree.GetRange(31, 99).GetIsEmpty());
    EXPECT_THROW(tree.GetRange(30, 10), InvalidArgument);

    AvlTree<int> empty;
    EXPECT_TRUE(empty.GetRange(0, 10).GetIsEmpty());
}

TEST(AvlTreeFind, CustomCompareAndStrings)
{
    AvlTree<int, Greater> descending;
    descending.Insert(1);
    descending.Insert(3);
    descending.Insert(2);
    EXPECT_EQ(descending.ToArraySequence(), (ArraySequence<int>{3, 2, 1}));

    AvlTree<std::string> names;
    names.Insert("Петров");
    names.Insert("Иванов");
    names.Insert("Сидоров");
    EXPECT_EQ(names.GetRange("Иванов", "Петров"),
              (ArraySequence<std::string>{"Иванов", "Петров"}));
}

// --- Случайные операции против std::set -------------------------------------

TEST(AvlTreeRandom, MatchesStdSetAndKeepsInvariants)
{
    std::mt19937 generator(123);
    std::uniform_int_distribution<int> valueDistribution(0, 299);
    std::uniform_int_distribution<int> actionDistribution(0, 2);

    AvlTree<int> tree;
    std::set<int> expected;
    for (int step = 0; step < 3000; ++step)
    {
        const int value = valueDistribution(generator);
        if (actionDistribution(generator) == 0)
        {
            if (expected.count(value) == 1)
            {
                tree.Remove(value);
                expected.erase(value);
            }
        }
        else
        {
            EXPECT_EQ(tree.Insert(value), expected.insert(value).second);
        }
        ASSERT_TRUE(tree.CheckInvariants()) << "после шага " << step;
    }

    const ArraySequence<int> items = tree.ToArraySequence();
    ASSERT_EQ(items.GetLength(), expected.size());
    std::size_t i = 0;
    for (int value : expected)
    {
        EXPECT_EQ(items.Get(i), value);
        ++i;
    }
}

// --- Копирование и перемещение ---------------------------------------------

TEST(AvlTreeCopy, CopyIsDeepAndIndependent)
{
    AvlTree<int> original = MakeTree({2, 1, 3});
    AvlTree<int> copy(original);
    copy.Insert(4);
    copy.Remove(1);
    EXPECT_EQ(original.ToArraySequence(), (ArraySequence<int>{1, 2, 3}));
    EXPECT_EQ(copy.ToArraySequence(), (ArraySequence<int>{2, 3, 4}));
    EXPECT_TRUE(copy.CheckInvariants());
}

TEST(AvlTreeCopy, AssignmentAndSelfAssignment)
{
    AvlTree<int> source = MakeTree({1, 2});
    AvlTree<int> target = MakeTree({7, 8, 9});
    target = source;
    EXPECT_EQ(target.ToArraySequence(), (ArraySequence<int>{1, 2}));

    AvlTree<int>& alias = target;
    target = alias;
    EXPECT_EQ(target.ToArraySequence(), (ArraySequence<int>{1, 2}));
    EXPECT_TRUE(target.CheckInvariants());
}

TEST(AvlTreeCopy, MoveLeavesSourceEmpty)
{
    AvlTree<int> source = MakeTree({1, 2, 3});
    AvlTree<int> target(std::move(source));
    EXPECT_EQ(target.GetCount(), 3u);
    EXPECT_TRUE(source.GetIsEmpty()); // NOLINT(bugprone-use-after-move): проверяем состояние

    AvlTree<int> other = MakeTree({9});
    other = std::move(target);
    EXPECT_EQ(other.ToArraySequence(), (ArraySequence<int>{1, 2, 3}));
}

TEST(AvlTreeCopy, ClearAndReuse)
{
    AvlTree<int> tree = MakeTree({3, 1, 2});
    tree.Clear();
    EXPECT_TRUE(tree.GetIsEmpty());
    EXPECT_EQ(tree.GetHeight(), 0);
    tree.Insert(5);
    EXPECT_EQ(tree.ToArraySequence(), (ArraySequence<int>{5}));
}

// --- Построитель ------------------------------------------------------------

TEST(TreeBuilderTest, BuildsFromSequenceSkippingDuplicates)
{
    TreeBuilder<int> builder;
    AvlTree<int> tree = builder.Build(ArraySequence<int>{5, 3, 5, 1, 3});
    EXPECT_EQ(tree.ToArraySequence(), (ArraySequence<int>{1, 3, 5}));
    EXPECT_TRUE(tree.CheckInvariants());
}

TEST(TreeBuilderTest, BuildsFromArrayAndRejectsNull)
{
    const int items[] = {4, 2, 6};
    TreeBuilder<int> builder;
    EXPECT_EQ(builder.Build(items, 3).ToArraySequence(), (ArraySequence<int>{2, 4, 6}));
    EXPECT_TRUE(builder.Build(nullptr, 0).GetIsEmpty());
    EXPECT_THROW(builder.Build(nullptr, 2), InvalidArgument);
}

TEST(TreeBuilderTest, UsesGivenCompare)
{
    TreeBuilder<int, Greater> builder;
    EXPECT_EQ(builder.Build(ArraySequence<int>{1, 3, 2}).ToArraySequence(),
              (ArraySequence<int>{3, 2, 1}));
}
