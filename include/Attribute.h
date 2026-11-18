#pragma once

// Атрибуты человека и составные атрибуты (DESIGN.md, п. 2.1, 4.5).
//
// Набор атрибутов фиксирован (задание, п. 2.5). Атрибут знает своё имя,
// вид значения и умеет получить значение у человека и разобрать значение,
// введённое пользователем. Возраст считается на заданный год, который
// хранится в самом атрибуте.

#include <cstddef>
#include <string>

#include "ArraySequence.h"
#include "AttributeValue.h"
#include "CompositeKey.h"
#include "Person.h"

enum class AttributeId
{
    Id,
    LastName,
    FirstName,
    MiddleName,
    FullName,
    Fio,
    BirthYear,
    Age
};

class Attribute
{
public:
    // Номер записи. Нужен контейнерам.
    Attribute();

    // Любой атрибут, кроме возраста. Для AttributeId::Age бросает
    // InvalidArgument: у возраста должен быть год, используйте AgeAt.
    explicit Attribute(AttributeId id);

    // Возраст на год year.
    static Attribute AgeAt(int year);

    AttributeId GetId() const;

    // Год расчёта возраста (только для возраста, иначе 0).
    int GetAgeYear() const;

    ValueKind GetKind() const;

    // «фамилия», «возраст на 2019 год» и т. п.
    std::string GetName() const;

    // Бросает InvalidArgument, если возраст нельзя вычислить
    // (год расчёта раньше года рождения).
    AttributeValue ExtractFrom(const Person& person) const;

    // Значение из текста, введённого пользователем. Для числовых атрибутов
    // текст должен быть целым числом, для строковых — непустым.
    // Бросает InvalidArgument с подсказкой.
    AttributeValue Parse(const std::string& text) const;

    // Все атрибуты, кроме возраста (у него есть параметр), — для меню.
    static ArraySequence<Attribute> GetPlainAttributes();

private:
    Attribute(AttributeId id, int ageYear);

    AttributeId id_;
    int ageYear_;
};

bool operator==(const Attribute& left, const Attribute& right);
bool operator!=(const Attribute& left, const Attribute& right);

// ---------------------------------------------------------------------------

// Составной атрибут: упорядоченный набор из одного или нескольких
// различных атрибутов. Строит ключи людей и проверяет ключи запросов.
class CompositeAttribute
{
public:
    // Атрибут «номер». Нужен контейнерам (ArraySequence создаёт массив
    // через new T[]), поэтому даёт корректный, а не пустой набор.
    CompositeAttribute();

    // Бросает InvalidArgument, если набор пуст или атрибут повторяется.
    explicit CompositeAttribute(const ArraySequence<Attribute>& parts);

    // Простой атрибут как составной из одного элемента.
    explicit CompositeAttribute(const Attribute& single);

    std::size_t GetSize() const;
    const Attribute& Get(std::size_t index) const;

    // «фамилия» или «фамилия + имя + год рождения».
    std::string GetName() const;

    CompositeKey MakeKey(const Person& person) const;

    // Ключ из текстов, по одному на каждый атрибут.
    // Бросает InvalidArgument, если число текстов не совпадает
    // с числом атрибутов или текст не разбирается.
    CompositeKey ParseKey(const ArraySequence<std::string>& texts) const;

    // Бросает InvalidArgument, если ключ не подходит к этому набору
    // атрибутов (другая длина или вид значений).
    void CheckKey(const CompositeKey& key) const;

private:
    ArraySequence<Attribute> parts_;
};
