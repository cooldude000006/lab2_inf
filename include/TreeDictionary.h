#pragma once

// Упорядоченный словарь на АВЛ-дереве — метод 2 (DESIGN.md, п. 3.3, 4.4).
//
// В дереве хранятся пары «ключ, значение», упорядоченные только по ключу
// (PairKeyLess). Для поиска строится пара-образец с нужным ключом и
// значением по умолчанию. Значение можно менять на месте через Get(key);
// ключ наружу доступен только для чтения, поэтому порядок дерева
// не нарушается.
//
// Сложность (n — число ключей, k — размер результата):
//   Get, ContainsKey, Add, Remove — O(log n);
//   GetRange — O(log n + k); GetKeys — O(n).
//
// Требования: K — operator<, копирование; V — конструктор по умолчанию,
// копирование и перемещение.

#include <cstddef>

#include "ArraySequence.h"
#include "AvlTree.h"
#include "IOrderedDictionary.h"
#include "Pair.h"

template <class K, class V>
class TreeDictionary : public IOrderedDictionary<K, V>
{
public:
    TreeDictionary() = default;

    // --- IDictionary ------------------------------------------------------

    std::size_t GetCount() const override;

    // У дерева нет заранее выделенного запаса: вместимость равна числу
    // элементов, каждый новый узел создаётся при вставке.
    std::size_t GetCapacity() const override;

    const V& Get(const K& key) const override;
    V& Get(const K& key) override;
    bool ContainsKey(const K& key) const override;
    void Add(const K& key, const V& value) override;
    void Remove(const K& key) override;

    // --- IOrderedDictionary -----------------------------------------------

    ArraySequence<Pair<K, V>> GetRange(const K& low, const K& high) const override;
    ArraySequence<K> GetKeys() const override;

    // Высота дерева — для замеров и просмотра в интерфейсе.
    int GetHeight() const;

private:
    using Entry = Pair<K, V>;
    using Tree = AvlTree<Entry, PairKeyLess<K, V>>;

    static Entry MakeProbe(const K& key);

    Tree tree_;
};

#include "TreeDictionary.tpp"
