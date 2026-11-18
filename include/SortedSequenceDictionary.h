#pragma once

// Упорядоченный словарь на отсортированной последовательности — метод 1
// (DESIGN.md, п. 3.3 «Метод 1», п. 4.4).
//
// Ключи хранятся в SortedSequence<K> (уникальные, по возрастанию),
// значения — в параллельном ArraySequence<V>: значение ключа с позицией i
// лежит в values_ на той же позиции i. Так значение можно менять на месте
// через Get(key), не нарушая порядка ключей, а сами ключи изменить снаружи
// нельзя.
//
// Сложность (n — число ключей, k — размер результата):
//   Get, ContainsKey — O(log n) (бинарный поиск);
//   Add, Remove — O(log n) + O(n) на сдвиг;
//   GetRange — O(log n + k).
//
// Требования: K — как у SortedSequence (operator<); V — как у ArraySequence.

#include <cstddef>

#include "ArraySequence.h"
#include "IOrderedDictionary.h"
#include "Pair.h"
#include "SortedSequence.h"

template <class K, class V>
class SortedSequenceDictionary : public IOrderedDictionary<K, V>
{
public:
    SortedSequenceDictionary() = default;

    // --- IDictionary ------------------------------------------------------

    std::size_t GetCount() const override;
    std::size_t GetCapacity() const override;
    const V& Get(const K& key) const override;
    V& Get(const K& key) override;
    bool ContainsKey(const K& key) const override;
    void Add(const K& key, const V& value) override;
    void Remove(const K& key) override;

    // --- IOrderedDictionary -----------------------------------------------

    ArraySequence<Pair<K, V>> GetRange(const K& low, const K& high) const override;
    ArraySequence<K> GetKeys() const override;

private:
    // Позиция ключа или ISortedSequence<K>::kNotFound.
    std::ptrdiff_t Find(const K& key) const;

    // Позиция ключа; бросает KeyNotFound, если ключа нет.
    std::size_t FindExisting(const K& key) const;

    SortedSequence<K> keys_;
    ArraySequence<V> values_;
};

#include "SortedSequenceDictionary.tpp"
