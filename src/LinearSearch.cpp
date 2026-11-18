#include "LinearSearch.h"

#include <utility>

#include "Exceptions.h"

ArraySequence<std::size_t> LinearFind(const ArraySequence<Person>& people,
                                      const CompositeAttribute& attribute,
                                      const CompositeKey& key)
{
    attribute.CheckKey(key);
    ArraySequence<std::size_t> positions;
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        if (attribute.MakeKey(people.Get(i)) == key)
        {
            positions.Append(i);
        }
    }
    return positions;
}

bool IsSortedByKey(const ArraySequence<Person>& people, const CompositeAttribute& attribute)
{
    if (people.GetLength() < 2)
    {
        return true;
    }
    CompositeKey previous = attribute.MakeKey(people.Get(0));
    for (std::size_t i = 1; i < people.GetLength(); ++i)
    {
        CompositeKey current = attribute.MakeKey(people.Get(i));
        if (current < previous)
        {
            return false;
        }
        previous = std::move(current);
    }
    return true;
}

ArraySequence<std::size_t> LinearFindRange(const ArraySequence<Person>& people,
                                           const CompositeAttribute& attribute,
                                           const CompositeKey& low, const CompositeKey& high)
{
    attribute.CheckKey(low);
    attribute.CheckKey(high);
    if (high < low)
    {
        throw InvalidArgument("Верхняя граница диапазона " + high.ToString()
                              + " меньше нижней " + low.ToString()
                              + "; поменяйте границы местами");
    }
    ArraySequence<std::size_t> positions;
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        const CompositeKey key = attribute.MakeKey(people.Get(i));
        if (low <= key && key <= high)
        {
            positions.Append(i);
        }
    }
    return positions;
}
