#pragma once

// Динамическая последовательность на массиве (DESIGN.md, п. 3.3, 4.1).
//
// Элементы лежат подряд в динамическом массиве с запасом ёмкости.
// Когда массив заполнен, ёмкость удваивается, поэтому добавление в конец
// в среднем стоит O(1). Вставка и удаление в середине сдвигают хвост: O(n).
// Доступ по индексу: O(1).
//
// Требования к T: конструктор по умолчанию (массив создаётся через
// new T[]), копирование и перемещение. Перемещение не должно бросать
// исключений, иначе при переезде массива возможна потеря данных.
//
// Индексы выходят за границы → IndexOutOfRange; некорректные аргументы
// (нулевой указатель, перевёрнутые границы) → InvalidArgument.
//
// Объявления — здесь, реализация шаблона — в ArraySequence.tpp,
// подключаемом в конце файла: определение шаблона должно быть видно
// в месте инстанцирования (методичка include-and-linking-and-templates).

#include <cstddef>
#include <initializer_list>

template <class T>
class ArraySequence
{
public:
    // --- Создание и уничтожение (правило пяти) ---------------------------

    ArraySequence();

    // Копирует count элементов из массива items.
    // Бросает InvalidArgument, если items == nullptr при count > 0.
    ArraySequence(const T* items, std::size_t count);

    ArraySequence(std::initializer_list<T> items);

    ArraySequence(const ArraySequence& other);
    ArraySequence(ArraySequence&& other) noexcept;

    // Присваивание через копирование и обмен: присваивание самому себе
    // безопасно (Голуб, правило 125).
    ArraySequence& operator=(const ArraySequence& other);
    ArraySequence& operator=(ArraySequence&& other) noexcept;

    ~ArraySequence();

    // --- Свойства ---------------------------------------------------------

    std::size_t GetLength() const;
    bool GetIsEmpty() const;

    // Сколько элементов поместится без перевыделения памяти.
    std::size_t GetCapacity() const;

    // --- Чтение -----------------------------------------------------------

    const T& Get(std::size_t index) const;

    // Доступ к элементу для изменения на месте (например, дописать
    // позицию в список позиций, который сам является элементом).
    T& Get(std::size_t index);

    const T& GetFirst() const;
    const T& GetLast() const;

    // Элементы с startIndex по endIndex включительно (как в спецификации
    // задания). Бросает InvalidArgument, если startIndex > endIndex,
    // и IndexOutOfRange, если endIndex >= GetLength().
    ArraySequence GetSubsequence(std::size_t startIndex, std::size_t endIndex) const;

    // --- Изменение --------------------------------------------------------

    void Set(std::size_t index, const T& value);

    void Append(const T& value);
    void Prepend(const T& value);

    // Вставляет value так, что он оказывается на позиции index.
    // index == GetLength() допустим: это вставка в конец.
    void InsertAt(const T& value, std::size_t index);

    void RemoveAt(std::size_t index);

    // Удаляет все элементы. Ёмкость сохраняется.
    void Clear();

    // Гарантирует ёмкость не меньше capacity. Длина не меняется.
    void Reserve(std::size_t capacity);

private:
    static constexpr std::size_t kMinCapacity = 4;
    static constexpr std::size_t kGrowthFactor = 2;

    void CheckIndex(std::size_t index) const;
    void Reallocate(std::size_t newCapacity);
    void Swap(ArraySequence& other) noexcept;

    T* items_;
    std::size_t length_;
    std::size_t capacity_;
};

// Последовательности равны, если у них одинаковая длина и попарно равные
// элементы. Требование к T: operator==.
template <class T>
bool operator==(const ArraySequence<T>& left, const ArraySequence<T>& right);

template <class T>
bool operator!=(const ArraySequence<T>& left, const ArraySequence<T>& right);

#include "ArraySequence.tpp"
