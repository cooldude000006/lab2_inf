#include "CompositeKey.h"

CompositeKey::CompositeKey()
    : values_()
{
}

CompositeKey::CompositeKey(const ArraySequence<AttributeValue>& values)
    : values_(values)
{
}

std::size_t CompositeKey::GetSize() const
{
    return values_.GetLength();
}

const AttributeValue& CompositeKey::Get(std::size_t index) const
{
    return values_.Get(index);
}

std::string CompositeKey::ToString() const
{
    if (values_.GetLength() == 1)
    {
        return values_.Get(0).ToString();
    }
    std::string text = "(";
    for (std::size_t i = 0; i < values_.GetLength(); ++i)
    {
        if (i > 0)
        {
            text += ", ";
        }
        text += values_.Get(i).ToString();
    }
    return text + ")";
}

bool operator<(const CompositeKey& left, const CompositeKey& right)
{
    const std::size_t common = left.GetSize() < right.GetSize() ? left.GetSize() : right.GetSize();
    for (std::size_t i = 0; i < common; ++i)
    {
        if (left.Get(i) < right.Get(i))
        {
            return true;
        }
        if (right.Get(i) < left.Get(i))
        {
            return false;
        }
    }
    return left.GetSize() < right.GetSize();
}

bool operator>(const CompositeKey& left, const CompositeKey& right)
{
    return right < left;
}

bool operator<=(const CompositeKey& left, const CompositeKey& right)
{
    return !(right < left);
}

bool operator>=(const CompositeKey& left, const CompositeKey& right)
{
    return !(left < right);
}

bool operator==(const CompositeKey& left, const CompositeKey& right)
{
    return !(left < right) && !(right < left);
}

bool operator!=(const CompositeKey& left, const CompositeKey& right)
{
    return !(left == right);
}
