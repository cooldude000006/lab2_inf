#pragma once

// Составной ключ — набор значений составного атрибута у одного человека
// (DESIGN.md, п. 2.1, 3.3 «Составной ключ»).
//
// Ключи сравниваются лексикографически: по первому значению, при равенстве
// по второму и так далее; если один ключ — начало другого, он меньше.
// Простой атрибут даёт ключ из одного значения.
//
// Используется своя структура, а не std::tuple: состав ключа выбирает
// пользователь во время работы программы.

#include <cstddef>
#include <string>

#include "ArraySequence.h"
#include "AttributeValue.h"

class CompositeKey
{
public:
    // Пустой ключ. Нужен контейнерам.
    CompositeKey();

    explicit CompositeKey(const ArraySequence<AttributeValue>& values);

    std::size_t GetSize() const;

    // Бросает IndexOutOfRange при неверном индексе.
    const AttributeValue& Get(std::size_t index) const;

    // «Иванов» для одного значения, «(Иванов, Пётр, 1995)» для нескольких.
    std::string ToString() const;

private:
    ArraySequence<AttributeValue> values_;
};

bool operator<(const CompositeKey& left, const CompositeKey& right);
bool operator>(const CompositeKey& left, const CompositeKey& right);
bool operator<=(const CompositeKey& left, const CompositeKey& right);
bool operator>=(const CompositeKey& left, const CompositeKey& right);
bool operator==(const CompositeKey& left, const CompositeKey& right);
bool operator!=(const CompositeKey& left, const CompositeKey& right);
