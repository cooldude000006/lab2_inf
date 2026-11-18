#pragma once

// Упорядоченный словарь: IDictionary с поиском по диапазону ключей
// (DESIGN.md, п. 3.3 «Интерфейсы словарей», п. 4.4).
//
// Индекс работает только через этот интерфейс, поэтому метод поиска
// (отсортированная последовательность или АВЛ-дерево) выбирается при
// построении индекса, а код поиска общий.
//
// Требование к K: operator< (строгий слабый порядок).

#include "ArraySequence.h"
#include "IDictionary.h"
#include "Pair.h"

template <class K, class V>
class IOrderedDictionary : public IDictionary<K, V>
{
public:
    // Пары с ключами из [low, high] (обе границы включаются) по возрастанию
    // ключа. Пустой результат, если таких ключей нет.
    // Бросает InvalidArgument, если high < low.
    virtual ArraySequence<Pair<K, V>> GetRange(const K& low, const K& high) const = 0;

    // Все ключи по возрастанию.
    virtual ArraySequence<K> GetKeys() const = 0;
};
