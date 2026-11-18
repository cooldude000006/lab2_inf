// Реализация шаблона ArraySequence. Подключается только из ArraySequence.h.

#include <string>
#include <utility>

#include "Exceptions.h"

// --- Создание и уничтожение ------------------------------------------------

template <class T>
ArraySequence<T>::ArraySequence()
    : items_(nullptr), length_(0), capacity_(0)
{
}

template <class T>
ArraySequence<T>::ArraySequence(const T* items, std::size_t count)
    : ArraySequence()
{
    if (items == nullptr && count > 0)
    {
        throw InvalidArgument("ArraySequence: передан нулевой указатель на массив из "
                              + std::to_string(count) + " элементов");
    }
    Reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        items_[i] = items[i];
    }
    length_ = count;
}

template <class T>
ArraySequence<T>::ArraySequence(std::initializer_list<T> items)
    : ArraySequence(items.begin(), items.size())
{
}

template <class T>
ArraySequence<T>::ArraySequence(const ArraySequence& other)
    : ArraySequence(other.items_, other.length_)
{
}

template <class T>
ArraySequence<T>::ArraySequence(ArraySequence&& other) noexcept
    : ArraySequence()
{
    Swap(other);
}

template <class T>
ArraySequence<T>& ArraySequence<T>::operator=(const ArraySequence& other)
{
    ArraySequence copy(other);
    Swap(copy);
    return *this;
}

template <class T>
ArraySequence<T>& ArraySequence<T>::operator=(ArraySequence&& other) noexcept
{
    ArraySequence taken(std::move(other));
    Swap(taken);
    return *this;
}

template <class T>
ArraySequence<T>::~ArraySequence()
{
    delete[] items_;
}

// --- Свойства --------------------------------------------------------------

template <class T>
std::size_t ArraySequence<T>::GetLength() const
{
    return length_;
}

template <class T>
bool ArraySequence<T>::GetIsEmpty() const
{
    return length_ == 0;
}

template <class T>
std::size_t ArraySequence<T>::GetCapacity() const
{
    return capacity_;
}

// --- Чтение ----------------------------------------------------------------

template <class T>
const T& ArraySequence<T>::Get(std::size_t index) const
{
    CheckIndex(index);
    return items_[index];
}

template <class T>
T& ArraySequence<T>::Get(std::size_t index)
{
    CheckIndex(index);
    return items_[index];
}

template <class T>
const T& ArraySequence<T>::GetFirst() const
{
    if (length_ == 0)
    {
        throw IndexOutOfRange("ArraySequence: нельзя взять первый элемент пустой "
                              "последовательности");
    }
    return items_[0];
}

template <class T>
const T& ArraySequence<T>::GetLast() const
{
    if (length_ == 0)
    {
        throw IndexOutOfRange("ArraySequence: нельзя взять последний элемент пустой "
                              "последовательности");
    }
    return items_[length_ - 1];
}

template <class T>
ArraySequence<T> ArraySequence<T>::GetSubsequence(std::size_t startIndex,
                                                  std::size_t endIndex) const
{
    if (startIndex > endIndex)
    {
        throw InvalidArgument("ArraySequence: начало подпоследовательности ("
                              + std::to_string(startIndex) + ") больше конца ("
                              + std::to_string(endIndex) + ")");
    }
    CheckIndex(endIndex);
    return ArraySequence(items_ + startIndex, endIndex - startIndex + 1);
}

// --- Изменение -------------------------------------------------------------

template <class T>
void ArraySequence<T>::Set(std::size_t index, const T& value)
{
    CheckIndex(index);
    items_[index] = value;
}

template <class T>
void ArraySequence<T>::Append(const T& value)
{
    InsertAt(value, length_);
}

template <class T>
void ArraySequence<T>::Prepend(const T& value)
{
    InsertAt(value, 0);
}

template <class T>
void ArraySequence<T>::InsertAt(const T& value, std::size_t index)
{
    if (index > length_)
    {
        throw IndexOutOfRange("ArraySequence: позиция вставки " + std::to_string(index)
                              + " больше длины " + std::to_string(length_));
    }

    // value может ссылаться на элемент этой же последовательности, который
    // сдвинется или переедет при перевыделении памяти. Копия делается до
    // любых изменений.
    T copy(value);

    if (length_ == capacity_)
    {
        Reallocate(capacity_ == 0 ? kMinCapacity : capacity_ * kGrowthFactor);
    }
    for (std::size_t i = length_; i > index; --i)
    {
        items_[i] = std::move(items_[i - 1]);
    }
    items_[index] = std::move(copy);
    ++length_;
}

template <class T>
void ArraySequence<T>::RemoveAt(std::size_t index)
{
    CheckIndex(index);
    for (std::size_t i = index; i + 1 < length_; ++i)
    {
        items_[i] = std::move(items_[i + 1]);
    }
    --length_;
    // Освободить ресурсы удалённого элемента (например, память строки).
    items_[length_] = T();
}

template <class T>
void ArraySequence<T>::Clear()
{
    for (std::size_t i = 0; i < length_; ++i)
    {
        items_[i] = T();
    }
    length_ = 0;
}

template <class T>
void ArraySequence<T>::Reserve(std::size_t capacity)
{
    if (capacity > capacity_)
    {
        Reallocate(capacity);
    }
}

// --- Закрытые методы -------------------------------------------------------

template <class T>
void ArraySequence<T>::CheckIndex(std::size_t index) const
{
    if (index >= length_)
    {
        throw IndexOutOfRange("ArraySequence: индекс " + std::to_string(index)
                              + " вне границ, длина " + std::to_string(length_));
    }
}

// Переносит элементы в новый массив ёмкостью newCapacity (newCapacity >= length_).
// Если выделить память не удалось, последовательность не меняется.
template <class T>
void ArraySequence<T>::Reallocate(std::size_t newCapacity)
{
    T* fresh = new T[newCapacity];
    for (std::size_t i = 0; i < length_; ++i)
    {
        fresh[i] = std::move(items_[i]);
    }
    delete[] items_;
    items_ = fresh;
    capacity_ = newCapacity;
}

template <class T>
void ArraySequence<T>::Swap(ArraySequence& other) noexcept
{
    std::swap(items_, other.items_);
    std::swap(length_, other.length_);
    std::swap(capacity_, other.capacity_);
}

// --- Сравнение -------------------------------------------------------------

template <class T>
bool operator==(const ArraySequence<T>& left, const ArraySequence<T>& right)
{
    if (left.GetLength() != right.GetLength())
    {
        return false;
    }
    for (std::size_t i = 0; i < left.GetLength(); ++i)
    {
        if (!(left.Get(i) == right.Get(i)))
        {
            return false;
        }
    }
    return true;
}

template <class T>
bool operator!=(const ArraySequence<T>& left, const ArraySequence<T>& right)
{
    return !(left == right);
}
