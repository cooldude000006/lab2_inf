#pragma once

// Отсортированная последовательность с бинарным поиском — метод 1
// (М-2; DESIGN.md, п. 3.3 «Метод 1», п. 4.2).
//
// Элементы хранятся в ArraySequence по возрастанию согласно правилу
// сравнения Compare. Все операции поиска выражаются через бинарный
// поиск границ:
//   LowerBound(x) — первая позиция, где элемент не меньше x;
//   UpperBound(x) — первая позиция, где элемент больше x.
// Элементы, равные x, занимают позиции [LowerBound(x), UpperBound(x)).
//
// Сложность (n — длина, k — размер результата):
//   LowerBound, UpperBound, IndexOf, Contains — O(log n);
//   GetRange — O(log n + k);
//   Add, RemoveAt — O(log n) на поиск места + O(n) на сдвиг хвоста.
//
// Равные элементы допускаются; новый элемент встаёт после равных ему,
// то есть порядок добавления равных элементов сохраняется.
//
// Требования к T: те же, что у ArraySequence. Compare — функциональный
// объект bool(const T&, const T&), задающий строгий слабый порядок;
// элементы a и b равны, если !compare(a, b) && !compare(b, a).

#include <cstddef>

#include "ArraySequence.h"
#include "Compare.h"
#include "ISortedSequence.h"

template <class T, class Compare = Less<T>>
class SortedSequence : public ISortedSequence<T>
{
public:
    // --- Создание ---------------------------------------------------------

    explicit SortedSequence(Compare compare = Compare());

    // Добавляет элементы items по одному через Add.
    explicit SortedSequence(const ArraySequence<T>& items, Compare compare = Compare());

    // --- ISortedSequence --------------------------------------------------

    std::size_t GetLength() const override;
    bool GetIsEmpty() const override;
    const T& Get(std::size_t index) const override;
    const T& GetFirst() const override;
    const T& GetLast() const override;
    std::ptrdiff_t IndexOf(const T& element) const override;
    void Add(const T& element) override;
    void RemoveAt(std::size_t index) override;

    // --- Бинарный поиск ---------------------------------------------------

    std::size_t LowerBound(const T& element) const;
    std::size_t UpperBound(const T& element) const;

    bool Contains(const T& element) const;

    // Элементы из [low, high] (обе границы включаются) по возрастанию.
    // Бросает InvalidArgument, если high < low.
    ArraySequence<T> GetRange(const T& low, const T& high) const;

    // --- Прочее -----------------------------------------------------------

    // Добавляет элемент и возвращает позицию, на которую он встал.
    // Нужен словарю, который хранит значения в параллельном массиве.
    std::size_t Insert(const T& element);

    // Элементы с startIndex по endIndex включительно (как в спецификации).
    // Исключения — как у ArraySequence::GetSubsequence.
    SortedSequence GetSubsequence(std::size_t startIndex, std::size_t endIndex) const;

    // Копия всех элементов по возрастанию.
    ArraySequence<T> ToArraySequence() const;

    // Ёмкость внутреннего массива.
    std::size_t GetCapacity() const;

    void Clear();

private:
    bool AreEqual(const T& left, const T& right) const;

    ArraySequence<T> items_;
    Compare compare_;
};

#include "SortedSequence.tpp"
