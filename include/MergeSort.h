#pragma once

// Устойчивая сортировка слиянием для ArraySequence.
//
// Нужна в двух местах прикладной логики:
// - индекс объединяет списки позиций из нескольких ключей диапазона
//   и выдаёт результат по возрастанию позиций;
// - генератор раскладывает созданные записи по возрастанию или убыванию
//   ключа для экспериментов (DESIGN.md, п. 3.3 «Генератор данных»).
// Саму коллекцию людей ничто не сортирует.
//
// Сложность O(n log n) в любом случае, дополнительная память O(n).
// Устойчивость: равные элементы сохраняют взаимный порядок.

#include <cstddef>
#include <utility>

#include "ArraySequence.h"
#include "Compare.h"

template <class T, class Compare = Less<T>>
void MergeSort(ArraySequence<T>& items, Compare compare = Compare());

// ---------------------------------------------------------------------------

namespace merge_sort_detail
{

// Сливает отсортированные половины items[begin, middle) и items[middle, end)
// через буфер. При равенстве берётся элемент левой половины — так
// сортировка остаётся устойчивой.
template <class T, class Compare>
void Merge(ArraySequence<T>& items, ArraySequence<T>& buffer, std::size_t begin,
           std::size_t middle, std::size_t end, Compare& compare)
{
    std::size_t left = begin;
    std::size_t right = middle;
    std::size_t out = begin;
    while (left < middle && right < end)
    {
        if (compare(items.Get(right), items.Get(left)))
        {
            buffer.Get(out) = std::move(items.Get(right));
            ++right;
        }
        else
        {
            buffer.Get(out) = std::move(items.Get(left));
            ++left;
        }
        ++out;
    }
    for (; left < middle; ++left, ++out)
    {
        buffer.Get(out) = std::move(items.Get(left));
    }
    for (; right < end; ++right, ++out)
    {
        buffer.Get(out) = std::move(items.Get(right));
    }
    for (std::size_t i = begin; i < end; ++i)
    {
        items.Get(i) = std::move(buffer.Get(i));
    }
}

template <class T, class Compare>
void SortRange(ArraySequence<T>& items, ArraySequence<T>& buffer, std::size_t begin,
               std::size_t end, Compare& compare)
{
    if (end - begin < 2)
    {
        return;
    }
    const std::size_t middle = begin + (end - begin) / 2;
    SortRange(items, buffer, begin, middle, compare);
    SortRange(items, buffer, middle, end, compare);
    Merge(items, buffer, begin, middle, end, compare);
}

} // namespace merge_sort_detail

template <class T, class Compare>
void MergeSort(ArraySequence<T>& items, Compare compare)
{
    // Буфер той же длины; его элементы перезаписываются при слиянии.
    ArraySequence<T> buffer = items;
    merge_sort_detail::SortRange(items, buffer, 0, items.GetLength(), compare);
}
