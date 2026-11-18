#pragma once

// Пара «ключ, значение» (DESIGN.md, п. 3.3).
//
// Это простая запись без поведения, поэтому struct с открытыми полями
// (Голуб, правило 114). Пары равны, если равны и ключи, и значения.
// Для упорядочивания по ключу используется отдельное правило PairKeyLess.

template <class K, class V>
struct Pair
{
    K key;
    V value;
};

template <class K, class V>
bool operator==(const Pair<K, V>& left, const Pair<K, V>& right);

template <class K, class V>
bool operator!=(const Pair<K, V>& left, const Pair<K, V>& right);

// Сравнение пар только по ключу: значения не учитываются.
// Требование к K: определён operator<.
template <class K, class V>
class PairKeyLess
{
public:
    bool operator()(const Pair<K, V>& left, const Pair<K, V>& right) const;
};

// ---------------------------------------------------------------------------

template <class K, class V>
bool operator==(const Pair<K, V>& left, const Pair<K, V>& right)
{
    return left.key == right.key && left.value == right.value;
}

template <class K, class V>
bool operator!=(const Pair<K, V>& left, const Pair<K, V>& right)
{
    return !(left == right);
}

template <class K, class V>
bool PairKeyLess<K, V>::operator()(const Pair<K, V>& left, const Pair<K, V>& right) const
{
    return left.key < right.key;
}
