#pragma once

// Линейный поиск — эталон (DESIGN.md, п. 3.3 «Линейный поиск», п. 4.7).
//
// Перебирает всю коллекцию, вычисляет ключ каждой записи и сравнивает его
// с запросом: O(n) на запрос, построение ничего не стоит. Результаты
// служат образцом для проверки бинарного поиска и АВЛ-дерева.
//
// Результат — позиции подходящих записей по возрастанию.
// Ключ, не подходящий к составному атрибуту, → InvalidArgument;
// high < low → InvalidArgument.

#include <cstddef>

#include "ArraySequence.h"
#include "Attribute.h"
#include "CompositeKey.h"
#include "Person.h"

ArraySequence<std::size_t> LinearFind(const ArraySequence<Person>& people,
                                      const CompositeAttribute& attribute,
                                      const CompositeKey& key);

// Идут ли записи по неубыванию ключа attribute. Нужна, чтобы заранее
// предупредить о построении отсортированной последовательности за O(n²).
// Бросает InvalidArgument, если у какой-то записи нельзя вычислить ключ.
bool IsSortedByKey(const ArraySequence<Person>& people, const CompositeAttribute& attribute);

ArraySequence<std::size_t> LinearFindRange(const ArraySequence<Person>& people,
                                           const CompositeAttribute& attribute,
                                           const CompositeKey& low, const CompositeKey& high);
