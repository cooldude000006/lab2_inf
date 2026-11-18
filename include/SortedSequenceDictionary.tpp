// Реализация шаблона SortedSequenceDictionary.
// Подключается только из SortedSequenceDictionary.h.

#include "Exceptions.h"

// --- IDictionary ------------------------------------------------------------

template <class K, class V>
std::size_t SortedSequenceDictionary<K, V>::GetCount() const
{
    return keys_.GetLength();
}

template <class K, class V>
std::size_t SortedSequenceDictionary<K, V>::GetCapacity() const
{
    return keys_.GetCapacity();
}

template <class K, class V>
const V& SortedSequenceDictionary<K, V>::Get(const K& key) const
{
    return values_.Get(FindExisting(key));
}

template <class K, class V>
V& SortedSequenceDictionary<K, V>::Get(const K& key)
{
    // Менять можно только значение: ключи лежат в keys_ и снаружи недоступны,
    // поэтому порядок словаря не нарушится.
    return values_.Get(FindExisting(key));
}

template <class K, class V>
bool SortedSequenceDictionary<K, V>::ContainsKey(const K& key) const
{
    return Find(key) != ISortedSequence<K>::kNotFound;
}

template <class K, class V>
void SortedSequenceDictionary<K, V>::Add(const K& key, const V& value)
{
    if (ContainsKey(key))
    {
        throw DuplicateKey("SortedSequenceDictionary: ключ уже есть в словаре; "
                           "чтобы изменить значение, используйте Get");
    }
    const std::size_t position = keys_.Insert(key);
    values_.InsertAt(value, position);
}

template <class K, class V>
void SortedSequenceDictionary<K, V>::Remove(const K& key)
{
    const std::size_t position = FindExisting(key);
    keys_.RemoveAt(position);
    values_.RemoveAt(position);
}

// --- IOrderedDictionary -----------------------------------------------------

template <class K, class V>
ArraySequence<Pair<K, V>> SortedSequenceDictionary<K, V>::GetRange(const K& low,
                                                                   const K& high) const
{
    if (high < low)
    {
        throw InvalidArgument("SortedSequenceDictionary: верхняя граница диапазона меньше "
                              "нижней; поменяйте границы местами");
    }
    const std::size_t begin = keys_.LowerBound(low);
    const std::size_t end = keys_.UpperBound(high);

    ArraySequence<Pair<K, V>> result;
    result.Reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i)
    {
        result.Append(Pair<K, V>{keys_.Get(i), values_.Get(i)});
    }
    return result;
}

template <class K, class V>
ArraySequence<K> SortedSequenceDictionary<K, V>::GetKeys() const
{
    return keys_.ToArraySequence();
}

// --- Закрытые методы --------------------------------------------------------

template <class K, class V>
std::ptrdiff_t SortedSequenceDictionary<K, V>::Find(const K& key) const
{
    return keys_.IndexOf(key);
}

template <class K, class V>
std::size_t SortedSequenceDictionary<K, V>::FindExisting(const K& key) const
{
    const std::ptrdiff_t position = Find(key);
    if (position == ISortedSequence<K>::kNotFound)
    {
        throw KeyNotFound("SortedSequenceDictionary: ключа нет в словаре");
    }
    return static_cast<std::size_t>(position);
}
