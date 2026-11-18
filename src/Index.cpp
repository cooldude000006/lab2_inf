#include "Index.h"

#include "Exceptions.h"
#include "LinearSearch.h"
#include "MergeSort.h"
#include "SortedSequenceDictionary.h"
#include "TreeDictionary.h"

Index::Index(const ArraySequence<Person>* people, const CompositeAttribute& attribute,
             SearchMethod method)
    : people_(people), attribute_(attribute), method_(method), dictionary_(nullptr)
{
    if (people == nullptr)
    {
        throw InvalidArgument("Index: не задана коллекция людей");
    }
    dictionary_ = CreateDictionary(method);
    if (dictionary_ == nullptr)
    {
        return; // линейному методу строить нечего
    }
    try
    {
        for (std::size_t i = 0; i < people_->GetLength(); ++i)
        {
            InsertPosition(i);
        }
    }
    catch (...)
    {
        // Деструктор не вызывается для не до конца созданного объекта,
        // поэтому словарь освобождается здесь.
        delete dictionary_;
        throw;
    }
}

Index::~Index()
{
    delete dictionary_;
}

const CompositeAttribute& Index::GetAttribute() const
{
    return attribute_;
}

SearchMethod Index::GetMethod() const
{
    return method_;
}

std::size_t Index::GetKeyCount() const
{
    return dictionary_ == nullptr ? 0 : dictionary_->GetCount();
}

Index::Positions Index::Find(const CompositeKey& key) const
{
    if (dictionary_ == nullptr)
    {
        return LinearFind(*people_, attribute_, key);
    }
    attribute_.CheckKey(key);
    if (!dictionary_->ContainsKey(key))
    {
        return Positions();
    }
    return dictionary_->Get(key);
}

Index::Positions Index::FindRange(const CompositeKey& low, const CompositeKey& high) const
{
    if (dictionary_ == nullptr)
    {
        return LinearFindRange(*people_, attribute_, low, high);
    }
    attribute_.CheckKey(low);
    attribute_.CheckKey(high);
    if (high < low)
    {
        throw InvalidArgument("Верхняя граница диапазона " + high.ToString()
                              + " меньше нижней " + low.ToString()
                              + "; поменяйте границы местами");
    }

    const ArraySequence<Pair<CompositeKey, Positions>> entries = dictionary_->GetRange(low, high);
    std::size_t total = 0;
    for (std::size_t i = 0; i < entries.GetLength(); ++i)
    {
        total += entries.Get(i).value.GetLength();
    }
    Positions positions;
    positions.Reserve(total);
    for (std::size_t i = 0; i < entries.GetLength(); ++i)
    {
        const Positions& part = entries.Get(i).value;
        for (std::size_t j = 0; j < part.GetLength(); ++j)
        {
            positions.Append(part.Get(j));
        }
    }
    // Позиции идут по ключам; общий порядок — по возрастанию позиций.
    MergeSort(positions);
    return positions;
}

void Index::AddPosition(std::size_t position)
{
    if (position >= people_->GetLength())
    {
        throw InvalidArgument("Index: позиции " + std::to_string(position)
                              + " нет в коллекции");
    }
    if (dictionary_ != nullptr)
    {
        InsertPosition(position);
    }
}

ArraySequence<Pair<CompositeKey, Index::Positions>> Index::GetEntries() const
{
    ArraySequence<Pair<CompositeKey, Positions>> entries;
    if (dictionary_ == nullptr)
    {
        return entries;
    }
    const ArraySequence<CompositeKey> keys = dictionary_->GetKeys();
    entries.Reserve(keys.GetLength());
    for (std::size_t i = 0; i < keys.GetLength(); ++i)
    {
        entries.Append(Pair<CompositeKey, Positions>{keys.Get(i), dictionary_->Get(keys.Get(i))});
    }
    return entries;
}

Index::Dictionary* Index::CreateDictionary(SearchMethod method)
{
    switch (method)
    {
        case SearchMethod::Linear:
            return nullptr;
        case SearchMethod::BinarySearch:
            return new SortedSequenceDictionary<CompositeKey, Positions>();
        case SearchMethod::Avl:
            return new TreeDictionary<CompositeKey, Positions>();
    }
    throw InvalidArgument("Index: неизвестный метод поиска");
}

// Позиции добавляются по возрастанию (сначала при построении, потом
// в конец коллекции), поэтому список позиций каждого ключа упорядочен.
void Index::InsertPosition(std::size_t position)
{
    const CompositeKey key = attribute_.MakeKey(people_->Get(position));
    if (dictionary_->ContainsKey(key))
    {
        dictionary_->Get(key).Append(position);
    }
    else
    {
        dictionary_->Add(key, Positions{position});
    }
}
