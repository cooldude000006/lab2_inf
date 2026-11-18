#include "Database.h"

#include <utility>

Database::Database()
    : people_(), indexes_(), maxId_(0)
{
}

Database::~Database()
{
    DeleteIndexes(indexes_);
}

// --- Коллекция --------------------------------------------------------------

const ArraySequence<Person>& Database::GetPeople() const
{
    return people_;
}

std::size_t Database::GetCount() const
{
    return people_.GetLength();
}

int Database::GetNextId() const
{
    return maxId_ + 1;
}

std::size_t Database::AddPerson(const Person& person)
{
    // Сначала проверка, что ключ человека вычисляется для всех индексов:
    // после добавления в коллекцию отступать уже поздно.
    for (std::size_t i = 0; i < indexes_.GetLength(); ++i)
    {
        indexes_.Get(i)->GetAttribute().MakeKey(person);
    }

    people_.Append(person);
    const std::size_t position = people_.GetLength() - 1;
    for (std::size_t i = 0; i < indexes_.GetLength(); ++i)
    {
        indexes_.Get(i)->AddPosition(position);
    }
    if (person.GetId() > maxId_)
    {
        maxId_ = person.GetId();
    }
    return position;
}

void Database::ReplacePeople(const ArraySequence<Person>& people)
{
    // Новые индексы строятся над новой коллекцией, пока старые ещё целы.
    // Коллекция подменяется на время построения, потому что индексы
    // хранят указатель именно на people_.
    ArraySequence<Person> oldPeople = std::move(people_);
    ArraySequence<Index*> rebuilt;
    try
    {
        people_ = people;
        for (std::size_t i = 0; i < indexes_.GetLength(); ++i)
        {
            const Index& old = *indexes_.Get(i);
            rebuilt.Append(nullptr);
            rebuilt.Get(i) = new Index(&people_, old.GetAttribute(), old.GetMethod());
        }
    }
    catch (...)
    {
        DeleteIndexes(rebuilt);
        people_ = std::move(oldPeople);
        throw;
    }

    DeleteIndexes(indexes_);
    indexes_ = std::move(rebuilt);
    maxId_ = FindMaxId(people_);
}

// --- Индексы ----------------------------------------------------------------

std::size_t Database::CreateIndex(const CompositeAttribute& attribute, SearchMethod method)
{
    // Место в списке резервируется заранее: если Append бросит исключение
    // после создания индекса, индекс потеряется.
    indexes_.Reserve(indexes_.GetLength() + 1);
    indexes_.Append(new Index(&people_, attribute, method));
    return indexes_.GetLength() - 1;
}

std::size_t Database::GetIndexCount() const
{
    return indexes_.GetLength();
}

const Index& Database::GetIndex(std::size_t number) const
{
    return *indexes_.Get(number);
}

void Database::RemoveIndex(std::size_t number)
{
    Index* index = indexes_.Get(number);
    indexes_.RemoveAt(number);
    delete index;
}

// --- Закрытые методы --------------------------------------------------------

void Database::DeleteIndexes(ArraySequence<Index*>& indexes)
{
    for (std::size_t i = 0; i < indexes.GetLength(); ++i)
    {
        delete indexes.Get(i);
    }
    indexes.Clear();
}

int Database::FindMaxId(const ArraySequence<Person>& people)
{
    int maxId = 0;
    for (std::size_t i = 0; i < people.GetLength(); ++i)
    {
        if (people.Get(i).GetId() > maxId)
        {
            maxId = people.Get(i).GetId();
        }
    }
    return maxId;
}
