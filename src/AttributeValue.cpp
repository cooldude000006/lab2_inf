#include "AttributeValue.h"

#include "Exceptions.h"

AttributeValue::AttributeValue()
    : kind_(ValueKind::Integer), integer_(0), string_()
{
}

AttributeValue::AttributeValue(int value)
    : kind_(ValueKind::Integer), integer_(value), string_()
{
}

AttributeValue::AttributeValue(const std::string& value)
    : kind_(ValueKind::String), integer_(0), string_(value)
{
}

ValueKind AttributeValue::GetKind() const
{
    return kind_;
}

int AttributeValue::GetInteger() const
{
    if (kind_ != ValueKind::Integer)
    {
        throw InvalidArgument("Значение атрибута «" + string_ + "» не является числом");
    }
    return integer_;
}

const std::string& AttributeValue::GetString() const
{
    if (kind_ != ValueKind::String)
    {
        throw InvalidArgument("Значение атрибута " + std::to_string(integer_)
                              + " не является строкой");
    }
    return string_;
}

std::string AttributeValue::ToString() const
{
    return kind_ == ValueKind::Integer ? std::to_string(integer_) : string_;
}

bool operator<(const AttributeValue& left, const AttributeValue& right)
{
    if (left.GetKind() != right.GetKind())
    {
        return left.GetKind() == ValueKind::Integer;
    }
    if (left.GetKind() == ValueKind::Integer)
    {
        return left.GetInteger() < right.GetInteger();
    }
    return left.GetString() < right.GetString();
}

bool operator>(const AttributeValue& left, const AttributeValue& right)
{
    return right < left;
}

bool operator<=(const AttributeValue& left, const AttributeValue& right)
{
    return !(right < left);
}

bool operator>=(const AttributeValue& left, const AttributeValue& right)
{
    return !(left < right);
}

bool operator==(const AttributeValue& left, const AttributeValue& right)
{
    return !(left < right) && !(right < left);
}

bool operator!=(const AttributeValue& left, const AttributeValue& right)
{
    return !(left == right);
}
