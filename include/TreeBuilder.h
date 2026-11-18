#pragma once

// Построение АВЛ-дерева из последовательности (DESIGN.md, п. 3.3 «Метод 2»).
//
// Вынесено в отдельный класс, а не в статический метод дерева, как
// рекомендует задание (пояснение 3b): так дерево не зависит от классов
// последовательностей, а новые источники данных добавляются сюда.
//
// Элементы вставляются по одному в порядке следования; повторы
// пропускаются. Благодаря балансировке построение занимает O(n log n)
// при любом порядке исходных данных.

#include <cstddef>

#include "ArraySequence.h"
#include "AvlTree.h"
#include "Compare.h"

template <class T, class Compare = Less<T>>
class TreeBuilder
{
public:
    explicit TreeBuilder(Compare compare = Compare());

    AvlTree<T, Compare> Build(const ArraySequence<T>& items) const;

    // Бросает InvalidArgument, если items == nullptr при count > 0.
    AvlTree<T, Compare> Build(const T* items, std::size_t count) const;

private:
    Compare compare_;
};

// ---------------------------------------------------------------------------

#include "Exceptions.h"

template <class T, class Compare>
TreeBuilder<T, Compare>::TreeBuilder(Compare compare)
    : compare_(compare)
{
}

template <class T, class Compare>
AvlTree<T, Compare> TreeBuilder<T, Compare>::Build(const ArraySequence<T>& items) const
{
    AvlTree<T, Compare> tree(compare_);
    for (std::size_t i = 0; i < items.GetLength(); ++i)
    {
        tree.Insert(items.Get(i));
    }
    return tree;
}

template <class T, class Compare>
AvlTree<T, Compare> TreeBuilder<T, Compare>::Build(const T* items, std::size_t count) const
{
    if (items == nullptr && count > 0)
    {
        throw InvalidArgument("TreeBuilder: передан нулевой указатель на массив");
    }
    AvlTree<T, Compare> tree(compare_);
    for (std::size_t i = 0; i < count; ++i)
    {
        tree.Insert(items[i]);
    }
    return tree;
}
