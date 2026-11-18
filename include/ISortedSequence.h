#pragma once

// Интерфейс отсортированной последовательности (спецификация SortedSequence
// из задания, п. 5 пояснений; DESIGN.md, п. 4.2).
//
// Элементы всегда упорядочены по возрастанию. Вместо Append, Prepend и
// InsertAt есть один метод Add, который сам находит место элемента.
// Равные элементы допускаются и стоят подряд.
//
// Отличия от спецификации задания:
// - индексы и длина имеют тип std::size_t, а не int;
// - GetSubsequence есть только у реализации (SortedSequence): интерфейс
//   не может вернуть по значению объект абстрактного типа;
// - добавлен RemoveAt: без удаления нельзя реализовать IDictionary::Remove.

#include <cstddef>

template <class T>
class ISortedSequence
{
public:
    // Значение IndexOf для отсутствующего элемента.
    static constexpr std::ptrdiff_t kNotFound = -1;

    virtual ~ISortedSequence() = default;

    virtual std::size_t GetLength() const = 0;
    virtual bool GetIsEmpty() const = 0;

    // Бросают IndexOutOfRange при неверном индексе или пустой последовательности.
    virtual const T& Get(std::size_t index) const = 0;
    virtual const T& GetFirst() const = 0;
    virtual const T& GetLast() const = 0;

    // Индекс первого элемента, равного element, или kNotFound.
    virtual std::ptrdiff_t IndexOf(const T& element) const = 0;

    // Добавляет элемент, сохраняя порядок.
    virtual void Add(const T& element) = 0;

    // Бросает IndexOutOfRange при неверном индексе.
    virtual void RemoveAt(std::size_t index) = 0;
};
