#pragma once

// Коллекция людей вместе с построенными над ней индексами
// (DESIGN.md, п. 3.3 «Индекс», п. 4.6, 4.8).
//
// Владеет коллекцией и индексами и следит, чтобы они были согласованы:
// - добавление человека дописывает его в конец коллекции и во все индексы;
// - замена всей коллекции (загрузка, генерация) перестраивает все индексы.
// Над одной коллекцией может быть сколько угодно индексов по разным
// атрибутам и методам.
//
// Объект нельзя копировать и перемещать: индексы хранят указатель
// на его коллекцию.

#include <cstddef>

#include "ArraySequence.h"
#include "Attribute.h"
#include "Index.h"
#include "Person.h"
#include "SearchMethod.h"

class Database
{
public:
    Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    ~Database();

    // --- Коллекция --------------------------------------------------------

    const ArraySequence<Person>& GetPeople() const;
    std::size_t GetCount() const;

    // Номер для новой записи: на 1 больше наибольшего номера в коллекции.
    int GetNextId() const;

    // Добавляет человека в конец коллекции и во все индексы; возвращает
    // его позицию. Если для какого-то индекса ключ человека вычислить
    // нельзя, бросает InvalidArgument и ничего не меняет.
    std::size_t AddPerson(const Person& person);

    // Заменяет коллекцию целиком и перестраивает индексы. Если какой-то
    // индекс построить нельзя, бросает исключение, и всё остаётся как было.
    void ReplacePeople(const ArraySequence<Person>& people);

    // --- Индексы ----------------------------------------------------------

    // Строит индекс и возвращает его номер.
    std::size_t CreateIndex(const CompositeAttribute& attribute, SearchMethod method);

    std::size_t GetIndexCount() const;

    // Бросают IndexOutOfRange при неверном номере.
    const Index& GetIndex(std::size_t number) const;
    void RemoveIndex(std::size_t number);

private:
    void DeleteIndexes(ArraySequence<Index*>& indexes);
    static int FindMaxId(const ArraySequence<Person>& people);

    ArraySequence<Person> people_;
    ArraySequence<Index*> indexes_;
    int maxId_;
};
