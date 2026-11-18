#pragma once

// Значение атрибута человека: целое число или строка
// (DESIGN.md, п. 2.1 «Атрибуты и ключи»).
//
// Порядок: числа сравниваются как числа, строки — побайтно (для UTF-8 это
// словарный порядок букв А–Я; буква Ё в Юникоде стоит перед А).
// Значения разных видов в одном ключе не встречаются: индекс проверяет
// ключ запроса. Чтобы порядок был полным, любое число считается меньше
// любой строки.

#include <string>

enum class ValueKind
{
    Integer,
    String
};

class AttributeValue
{
public:
    // Целое 0. Нужен контейнерам.
    AttributeValue();

    explicit AttributeValue(int value);
    explicit AttributeValue(const std::string& value);

    ValueKind GetKind() const;

    // Бросают InvalidArgument, если значение другого вида.
    int GetInteger() const;
    const std::string& GetString() const;

    // Текст для вывода: число цифрами, строка как есть.
    std::string ToString() const;

private:
    ValueKind kind_;
    int integer_;
    std::string string_;
};

bool operator<(const AttributeValue& left, const AttributeValue& right);
bool operator>(const AttributeValue& left, const AttributeValue& right);
bool operator<=(const AttributeValue& left, const AttributeValue& right);
bool operator>=(const AttributeValue& left, const AttributeValue& right);
bool operator==(const AttributeValue& left, const AttributeValue& right);
bool operator!=(const AttributeValue& left, const AttributeValue& right);
