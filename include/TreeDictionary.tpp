// Реализация шаблона TreeDictionary. Подключается только из TreeDictionary.h.

#include "Exceptions.h"

// --- IDictionary ------------------------------------------------------------

template <class K, class V>
std::size_t TreeDictionary<K, V>::GetCount() const
{
    return tree_.GetCount();
}

template <class K, class V>
std::size_t TreeDictionary<K, V>::GetCapacity() const
{
    return tree_.GetCount();
}

template <class K, class V>
const V& TreeDictionary<K, V>::Get(const K& key) const
{
    const Entry* entry = tree_.Find(MakeProbe(key));
    if (entry == nullptr)
    {
        throw KeyNotFound("TreeDictionary: ключа нет в словаре");
    }
    return entry->value;
}

template <class K, class V>
V& TreeDictionary<K, V>::Get(const K& key)
{
    // Сравнение идёт только по ключу, поэтому менять значение безопасно.
    Entry* entry = tree_.FindForUpdate(MakeProbe(key));
    if (entry == nullptr)
    {
        throw KeyNotFound("TreeDictionary: ключа нет в словаре");
    }
    return entry->value;
}

template <class K, class V>
bool TreeDictionary<K, V>::ContainsKey(const K& key) const
{
    return tree_.Contains(MakeProbe(key));
}

template <class K, class V>
void TreeDictionary<K, V>::Add(const K& key, const V& value)
{
    if (!tree_.Insert(Entry{key, value}))
    {
        throw DuplicateKey("TreeDictionary: ключ уже есть в словаре; "
                           "чтобы изменить значение, используйте Get");
    }
}

template <class K, class V>
void TreeDictionary<K, V>::Remove(const K& key)
{
    if (!tree_.Contains(MakeProbe(key)))
    {
        throw KeyNotFound("TreeDictionary: ключа нет в словаре");
    }
    tree_.Remove(MakeProbe(key));
}

// --- IOrderedDictionary -----------------------------------------------------

template <class K, class V>
ArraySequence<Pair<K, V>> TreeDictionary<K, V>::GetRange(const K& low, const K& high) const
{
    if (high < low)
    {
        throw InvalidArgument("TreeDictionary: верхняя граница диапазона меньше нижней; "
                              "поменяйте границы местами");
    }
    return tree_.GetRange(MakeProbe(low), MakeProbe(high));
}

template <class K, class V>
ArraySequence<K> TreeDictionary<K, V>::GetKeys() const
{
    const ArraySequence<Entry> entries = tree_.ToArraySequence();
    ArraySequence<K> keys;
    keys.Reserve(entries.GetLength());
    for (std::size_t i = 0; i < entries.GetLength(); ++i)
    {
        keys.Append(entries.Get(i).key);
    }
    return keys;
}

template <class K, class V>
int TreeDictionary<K, V>::GetHeight() const
{
    return tree_.GetHeight();
}

// --- Закрытые методы --------------------------------------------------------

template <class K, class V>
typename TreeDictionary<K, V>::Entry TreeDictionary<K, V>::MakeProbe(const K& key)
{
    return Entry{key, V()};
}
