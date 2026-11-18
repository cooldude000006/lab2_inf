#pragma once

// Вспомогательные функции для тестов.

#include <gtest/gtest.h>
#include <ostream>

#include "ArraySequence.h"
#include "AttributeValue.h"
#include "CompositeKey.h"
#include "Pair.h"
#include "Person.h"

inline void PrintTo(const AttributeValue& value, std::ostream* out)
{
    *out << value.ToString();
}

inline void PrintTo(const CompositeKey& key, std::ostream* out)
{
    *out << key.ToString();
}

inline void PrintTo(const Person& person, std::ostream* out)
{
    *out << "#" << person.GetId() << " " << person.GetFullName() << " " << person.GetBirthYear();
}

// Печать пары и последовательности в сообщениях GoogleTest о провале
// проверки, чтобы видеть элементы, а не байты объекта.
template <class K, class V>
void PrintTo(const Pair<K, V>& pair, std::ostream* out)
{
    *out << "(" << ::testing::PrintToString(pair.key) << " -> "
         << ::testing::PrintToString(pair.value) << ")";
}

template <class T>
void PrintTo(const ArraySequence<T>& sequence, std::ostream* out)
{
    *out << "{";
    for (std::size_t i = 0; i < sequence.GetLength(); ++i)
    {
        if (i > 0)
        {
            *out << ", ";
        }
        *out << ::testing::PrintToString(sequence.Get(i));
    }
    *out << "}";
}
