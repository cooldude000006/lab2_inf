#pragma once

// Интерфейс словаря (спецификация IDictionary из задания, п. 8 пояснений;
// DESIGN.md, п. 3.3, 4.4).
//
// Хранит пары «ключ → значение» с уникальными ключами.
//
// Отличия от спецификации задания:
// - количество и вместимость имеют тип std::size_t, а не int;
// - Get есть в двух вариантах: для чтения и для изменения значения на
//   месте. Изменяемый вариант нужен индексу, чтобы дописывать позицию
//   в список позиций уже существующего ключа без удаления и повторного
//   добавления.

#include <cstddef>

template <class K, class V>
class IDictionary
{
public:
    virtual ~IDictionary() = default;

    // Число хранимых элементов.
    virtual std::size_t GetCount() const = 0;

    // Сколько элементов поместится без перестройки внутренней структуры.
    virtual std::size_t GetCapacity() const = 0;

    // Значение по ключу. Бросает KeyNotFound, если ключа нет.
    virtual const V& Get(const K& key) const = 0;
    virtual V& Get(const K& key) = 0;

    virtual bool ContainsKey(const K& key) const = 0;

    // Бросает DuplicateKey, если ключ уже есть.
    virtual void Add(const K& key, const V& value) = 0;

    // Бросает KeyNotFound, если ключа нет.
    virtual void Remove(const K& key) = 0;
};
