#pragma once

// Генератор случайных людей (DESIGN.md, п. 3.3 «Генератор данных», п. 4.8).
//
// Фамилии, имена и отчества берутся из встроенных списков, пол выбирается
// случайно и согласуется с формой фамилии и отчества. Год рождения —
// равномерно из заданного интервала. Номера записей — 1, 2, ..., n
// в порядке создания.
//
// Для экспериментов (сноска 1 задания) генератор выдаёт записи в нужном
// порядке по выбранному атрибуту: по возрастанию, по убыванию или
// в случайном порядке. Это подготовка исходных данных: созданную
// коллекцию программа потом не переставляет.
//
// Одинаковое начальное значение (seed) даёт одинаковые данные — замеры
// можно повторить.

#include <cstddef>
#include <random>
#include <string>

#include "ArraySequence.h"
#include "Attribute.h"
#include "Person.h"

enum class DataOrder
{
    Ascending,
    Descending,
    Random,
    AsIs // данные пользователя в том порядке, как есть (для сравнения на них)
};

std::string GetOrderName(DataOrder order);

// Порядки для экспериментов: по возрастанию, по убыванию, случайный.
ArraySequence<DataOrder> GetAllDataOrders();

class PersonGenerator
{
public:
    explicit PersonGenerator(unsigned seed);

    // count записей в случайном порядке с годами рождения из
    // [minYear, maxYear]. Бросает InvalidArgument, если minYear > maxYear
    // или интервал выходит за допустимые годы рождения.
    ArraySequence<Person> Generate(std::size_t count, int minYear, int maxYear);

    // Те же записи, разложенные по ключу attribute: по возрастанию,
    // по убыванию или перемешанные случайно (равновероятная перестановка).
    // При упорядочивании записи с равными ключами идут в порядке создания.
    // AsIs — порядок создания, как у Generate.
    ArraySequence<Person> GenerateOrdered(std::size_t count, int minYear, int maxYear,
                                          const CompositeAttribute& attribute, DataOrder order);

private:
    Person MakePerson(int id, int minYear, int maxYear);
    std::size_t RandomIndex(std::size_t size);

    std::mt19937 random_;
};
