// Реализация шаблона SortedSequence. Подключается только из SortedSequence.h.

#include "Exceptions.h"

// --- Создание ---------------------------------------------------------------

template <class T, class Compare>
SortedSequence<T, Compare>::SortedSequence(Compare compare)
    : items_(), compare_(compare)
{
}

template <class T, class Compare>
SortedSequence<T, Compare>::SortedSequence(const ArraySequence<T>& items, Compare compare)
    : SortedSequence(compare)
{
    items_.Reserve(items.GetLength());
    for (std::size_t i = 0; i < items.GetLength(); ++i)
    {
        Add(items.Get(i));
    }
}

// --- ISortedSequence --------------------------------------------------------

template <class T, class Compare>
std::size_t SortedSequence<T, Compare>::GetLength() const
{
    return items_.GetLength();
}

template <class T, class Compare>
bool SortedSequence<T, Compare>::GetIsEmpty() const
{
    return items_.GetIsEmpty();
}

template <class T, class Compare>
const T& SortedSequence<T, Compare>::Get(std::size_t index) const
{
    return items_.Get(index);
}

template <class T, class Compare>
const T& SortedSequence<T, Compare>::GetFirst() const
{
    return items_.GetFirst();
}

template <class T, class Compare>
const T& SortedSequence<T, Compare>::GetLast() const
{
    return items_.GetLast();
}

template <class T, class Compare>
std::ptrdiff_t SortedSequence<T, Compare>::IndexOf(const T& element) const
{
    const std::size_t position = LowerBound(element);
    if (position < items_.GetLength() && AreEqual(items_.Get(position), element))
    {
        return static_cast<std::ptrdiff_t>(position);
    }
    return ISortedSequence<T>::kNotFound;
}

template <class T, class Compare>
void SortedSequence<T, Compare>::Add(const T& element)
{
    Insert(element);
}

template <class T, class Compare>
void SortedSequence<T, Compare>::RemoveAt(std::size_t index)
{
    items_.RemoveAt(index);
}

// --- Бинарный поиск ---------------------------------------------------------

// Инвариант цикла: все элементы левее left меньше element,
// все элементы начиная с right — не меньше. Отрезок [left, right)
// на каждом шаге сокращается вдвое, поэтому шагов не больше log2(n) + 1.
template <class T, class Compare>
std::size_t SortedSequence<T, Compare>::LowerBound(const T& element) const
{
    std::size_t left = 0;
    std::size_t right = items_.GetLength();
    while (left < right)
    {
        // Так не бывает переполнения, в отличие от (left + right) / 2.
        const std::size_t middle = left + (right - left) / 2;
        if (compare_(items_.Get(middle), element))
        {
            left = middle + 1;
        }
        else
        {
            right = middle;
        }
    }
    return left;
}

// Инвариант цикла: все элементы левее left не больше element,
// все элементы начиная с right — больше.
template <class T, class Compare>
std::size_t SortedSequence<T, Compare>::UpperBound(const T& element) const
{
    std::size_t left = 0;
    std::size_t right = items_.GetLength();
    while (left < right)
    {
        const std::size_t middle = left + (right - left) / 2;
        if (compare_(element, items_.Get(middle)))
        {
            right = middle;
        }
        else
        {
            left = middle + 1;
        }
    }
    return left;
}

template <class T, class Compare>
bool SortedSequence<T, Compare>::Contains(const T& element) const
{
    return IndexOf(element) != ISortedSequence<T>::kNotFound;
}

template <class T, class Compare>
ArraySequence<T> SortedSequence<T, Compare>::GetRange(const T& low, const T& high) const
{
    if (compare_(high, low))
    {
        throw InvalidArgument("SortedSequence: верхняя граница диапазона меньше нижней; "
                              "поменяйте границы местами");
    }
    const std::size_t begin = LowerBound(low);
    const std::size_t end = UpperBound(high);

    ArraySequence<T> result;
    result.Reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i)
    {
        result.Append(items_.Get(i));
    }
    return result;
}

// --- Прочее -----------------------------------------------------------------

template <class T, class Compare>
std::size_t SortedSequence<T, Compare>::Insert(const T& element)
{
    // После равных элементов: порядок добавления равных сохраняется,
    // а на возрастающих данных вставка всегда идёт в конец без сдвигов.
    const std::size_t position = UpperBound(element);
    items_.InsertAt(element, position);
    return position;
}

template <class T, class Compare>
SortedSequence<T, Compare> SortedSequence<T, Compare>::GetSubsequence(std::size_t startIndex,
                                                                      std::size_t endIndex) const
{
    // Часть отсортированной последовательности уже отсортирована,
    // поэтому элементы копируются без повторной вставки.
    SortedSequence result(compare_);
    result.items_ = items_.GetSubsequence(startIndex, endIndex);
    return result;
}

template <class T, class Compare>
ArraySequence<T> SortedSequence<T, Compare>::ToArraySequence() const
{
    return items_;
}

template <class T, class Compare>
std::size_t SortedSequence<T, Compare>::GetCapacity() const
{
    return items_.GetCapacity();
}

template <class T, class Compare>
void SortedSequence<T, Compare>::Clear()
{
    items_.Clear();
}

// --- Закрытые методы --------------------------------------------------------

template <class T, class Compare>
bool SortedSequence<T, Compare>::AreEqual(const T& left, const T& right) const
{
    return !compare_(left, right) && !compare_(right, left);
}
