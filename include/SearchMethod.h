#pragma once

// Методы поиска (DESIGN.md, п. 1.1, 2.1 «Поиск»).

#include <string>

#include "ArraySequence.h"

enum class SearchMethod
{
    Linear,       // перебор коллекции, индекса нет (эталон)
    BinarySearch, // отсортированная последовательность ключей
    Avl           // АВЛ-дерево
};

std::string GetMethodName(SearchMethod method);

// Все методы в порядке перечисления — для меню и замеров.
ArraySequence<SearchMethod> GetAllSearchMethods();
