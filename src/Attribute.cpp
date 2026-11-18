#include "Attribute.h"

#include "Exceptions.h"
#include "TextUtils.h"

// --- Attribute --------------------------------------------------------------

Attribute::Attribute()
    : id_(AttributeId::Id), ageYear_(0)
{
}

Attribute::Attribute(AttributeId id)
    : id_(id), ageYear_(0)
{
    if (id == AttributeId::Age)
    {
        throw InvalidArgument("Для возраста нужно указать год расчёта");
    }
}

Attribute::Attribute(AttributeId id, int ageYear)
    : id_(id), ageYear_(ageYear)
{
}

Attribute Attribute::AgeAt(int year)
{
    if (year < Person::kMinBirthYear)
    {
        throw InvalidArgument("Год расчёта возраста должен быть не раньше "
                              + std::to_string(Person::kMinBirthYear));
    }
    return Attribute(AttributeId::Age, year);
}

AttributeId Attribute::GetId() const
{
    return id_;
}

int Attribute::GetAgeYear() const
{
    return ageYear_;
}

ValueKind Attribute::GetKind() const
{
    switch (id_)
    {
        case AttributeId::Id:
        case AttributeId::BirthYear:
        case AttributeId::Age:
            return ValueKind::Integer;
        default:
            return ValueKind::String;
    }
}

std::string Attribute::GetName() const
{
    switch (id_)
    {
        case AttributeId::Id:
            return "номер";
        case AttributeId::LastName:
            return "фамилия";
        case AttributeId::FirstName:
            return "имя";
        case AttributeId::MiddleName:
            return "отчество";
        case AttributeId::FullName:
            return "полное имя";
        case AttributeId::Fio:
            return "фамилия И.О.";
        case AttributeId::BirthYear:
            return "год рождения";
        case AttributeId::Age:
            return "возраст на " + std::to_string(ageYear_) + " год";
    }
    return "?";
}

AttributeValue Attribute::ExtractFrom(const Person& person) const
{
    switch (id_)
    {
        case AttributeId::Id:
            return AttributeValue(person.GetId());
        case AttributeId::LastName:
            return AttributeValue(person.GetLastName());
        case AttributeId::FirstName:
            return AttributeValue(person.GetFirstName());
        case AttributeId::MiddleName:
            return AttributeValue(person.GetMiddleName());
        case AttributeId::FullName:
            return AttributeValue(person.GetFullName());
        case AttributeId::Fio:
            return AttributeValue(person.GetFio());
        case AttributeId::BirthYear:
            return AttributeValue(person.GetBirthYear());
        case AttributeId::Age:
            return AttributeValue(person.GetAge(ageYear_));
    }
    throw InvalidArgument("Неизвестный атрибут");
}

AttributeValue Attribute::Parse(const std::string& text) const
{
    if (GetKind() == ValueKind::Integer)
    {
        return AttributeValue(ParseInteger(text, GetName()));
    }
    const std::string trimmed = Trim(text);
    if (trimmed.empty())
    {
        throw InvalidArgument(GetName() + ": значение не задано, введите текст");
    }
    return AttributeValue(trimmed);
}

ArraySequence<Attribute> Attribute::GetPlainAttributes()
{
    return ArraySequence<Attribute>{
        Attribute(AttributeId::Id),         Attribute(AttributeId::LastName),
        Attribute(AttributeId::FirstName),  Attribute(AttributeId::MiddleName),
        Attribute(AttributeId::FullName),   Attribute(AttributeId::Fio),
        Attribute(AttributeId::BirthYear)};
}

bool operator==(const Attribute& left, const Attribute& right)
{
    return left.GetId() == right.GetId() && left.GetAgeYear() == right.GetAgeYear();
}

bool operator!=(const Attribute& left, const Attribute& right)
{
    return !(left == right);
}

// --- CompositeAttribute -----------------------------------------------------

CompositeAttribute::CompositeAttribute()
    : parts_{Attribute(AttributeId::Id)}
{
}

CompositeAttribute::CompositeAttribute(const ArraySequence<Attribute>& parts)
    : parts_(parts)
{
    if (parts.GetIsEmpty())
    {
        throw InvalidArgument("Выберите хотя бы один атрибут для индекса");
    }
    for (std::size_t i = 0; i < parts.GetLength(); ++i)
    {
        for (std::size_t j = i + 1; j < parts.GetLength(); ++j)
        {
            if (parts.Get(i) == parts.Get(j))
            {
                throw InvalidArgument("Атрибут «" + parts.Get(i).GetName()
                                      + "» выбран дважды; уберите повтор");
            }
        }
    }
}

CompositeAttribute::CompositeAttribute(const Attribute& single)
    : CompositeAttribute(ArraySequence<Attribute>{single})
{
}

std::size_t CompositeAttribute::GetSize() const
{
    return parts_.GetLength();
}

const Attribute& CompositeAttribute::Get(std::size_t index) const
{
    return parts_.Get(index);
}

std::string CompositeAttribute::GetName() const
{
    std::string name;
    for (std::size_t i = 0; i < parts_.GetLength(); ++i)
    {
        if (i > 0)
        {
            name += " + ";
        }
        name += parts_.Get(i).GetName();
    }
    return name;
}

CompositeKey CompositeAttribute::MakeKey(const Person& person) const
{
    ArraySequence<AttributeValue> values;
    values.Reserve(parts_.GetLength());
    for (std::size_t i = 0; i < parts_.GetLength(); ++i)
    {
        values.Append(parts_.Get(i).ExtractFrom(person));
    }
    return CompositeKey(values);
}

CompositeKey CompositeAttribute::ParseKey(const ArraySequence<std::string>& texts) const
{
    if (texts.GetLength() != parts_.GetLength())
    {
        throw InvalidArgument("Для ключа «" + GetName() + "» нужно "
                              + std::to_string(parts_.GetLength()) + " значений, получено "
                              + std::to_string(texts.GetLength()));
    }
    ArraySequence<AttributeValue> values;
    values.Reserve(parts_.GetLength());
    for (std::size_t i = 0; i < parts_.GetLength(); ++i)
    {
        values.Append(parts_.Get(i).Parse(texts.Get(i)));
    }
    return CompositeKey(values);
}

void CompositeAttribute::CheckKey(const CompositeKey& key) const
{
    if (key.GetSize() != parts_.GetLength())
    {
        throw InvalidArgument("Ключ " + key.ToString() + " не подходит к индексу «" + GetName()
                              + "»: нужно значений " + std::to_string(parts_.GetLength()));
    }
    for (std::size_t i = 0; i < parts_.GetLength(); ++i)
    {
        if (key.Get(i).GetKind() != parts_.Get(i).GetKind())
        {
            throw InvalidArgument("Ключ " + key.ToString() + ": значение «"
                                  + key.Get(i).ToString() + "» не подходит к атрибуту «"
                                  + parts_.Get(i).GetName() + "»");
        }
    }
}
