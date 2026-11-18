#pragma once

// Индекс над коллекцией людей (DESIGN.md, п. 2.1 «Поиск», п. 3.3 «Индекс»,
// п. 4.6).
//
// Связывает ключ составного атрибута с позициями всех записей, у которых
// этот ключ. Устройство задаётся методом поиска:
// - BinarySearch — словарь на отсортированной последовательности;
// - Avl — словарь на АВЛ-дереве;
// - Linear — словаря нет, запросы выполняет линейный поиск по коллекции.
// Код запросов один для всех методов: работа идёт через IOrderedDictionary.
//
// Индекс строится в конструкторе, поэтому время построения — это время
// создания объекта. Сама коллекция не меняется и не сортируется.
//
// Результат любого запроса — позиции записей по возрастанию; так
// результаты разных методов можно сравнивать простым равенством.
//
// Индекс хранит указатель на коллекцию и не владеет ею. Коллекция должна
// жить дольше индекса; при добавлении записи в конец коллекции владелец
// вызывает AddPosition (это делает Database).

#include <cstddef>

#include "ArraySequence.h"
#include "Attribute.h"
#include "CompositeKey.h"
#include "IOrderedDictionary.h"
#include "Pair.h"
#include "Person.h"
#include "SearchMethod.h"

class Index
{
public:
    using Positions = ArraySequence<std::size_t>;

    // Строит индекс. Бросает InvalidArgument, если people == nullptr или
    // у какой-то записи нельзя вычислить ключ (например, возраст на год
    // раньше года рождения).
    Index(const ArraySequence<Person>* people, const CompositeAttribute& attribute,
          SearchMethod method);

    // Индекс владеет словарём; копировать его незачем (Голуб, правило 126:
    // либо определить копирование, либо запретить).
    Index(const Index&) = delete;
    Index& operator=(const Index&) = delete;

    ~Index();

    const CompositeAttribute& GetAttribute() const;
    SearchMethod GetMethod() const;

    // Число различных ключей (0 для линейного метода — у него нет словаря).
    std::size_t GetKeyCount() const;

    // Позиции записей с ключом key. Пусто, если таких нет.
    // Бросает InvalidArgument, если ключ не подходит к атрибуту индекса.
    Positions Find(const CompositeKey& key) const;

    // Позиции записей с ключом из [low, high].
    // Бросает InvalidArgument, если ключ не подходит или high < low.
    Positions FindRange(const CompositeKey& low, const CompositeKey& high) const;

    // Учесть запись, добавленную в коллекцию на позицию position.
    // Бросает InvalidArgument, если такой позиции в коллекции нет.
    void AddPosition(std::size_t position);

    // Все ключи по возрастанию с позициями — для просмотра индекса.
    // Для линейного метода пусто.
    ArraySequence<Pair<CompositeKey, Positions>> GetEntries() const;

private:
    using Dictionary = IOrderedDictionary<CompositeKey, Positions>;

    static Dictionary* CreateDictionary(SearchMethod method);
    void InsertPosition(std::size_t position);

    const ArraySequence<Person>* people_;
    CompositeAttribute attribute_;
    SearchMethod method_;
    Dictionary* dictionary_;
};
