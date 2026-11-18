#pragma once

// Замеры и сравнение методов поиска (DESIGN.md, п. 3.3 «Замеры», п. 4.9).
//
// Эксперимент перебирает атрибуты, размеры коллекции, порядок исходных
// данных и методы. Для каждого сочетания:
// 1. генератор создаёт коллекцию в нужном порядке по атрибуту
//    (одни и те же данные для всех методов);
// 2. индекс строится repeats раз, в результат идёт медиана времени
//    построения (построение дольше секунды не повторяется);
// 3. на построенном индексе выполняется серия запросов по ключу
//    (ключи существующих записей) и по диапазону (узкие диапазоны между
//    соседними ключами случайной выборки), берётся среднее время запроса.
//
// Построение отсортированной последовательности на данных по убыванию и
// случайных занимает O(n²); для размеров больше quadraticSizeLimit такие
// замеры пропускаются, чтобы эксперимент не шёл часами.

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>

#include "ArraySequence.h"
#include "Attribute.h"
#include "PersonGenerator.h"
#include "SearchMethod.h"

// Настройки эксперимента. Простая запись без поведения (Голуб, правило 114).
struct ExperimentConfig
{
    ArraySequence<CompositeAttribute> attributes;
    ArraySequence<std::size_t> sizes;
    ArraySequence<DataOrder> orders;
    ArraySequence<SearchMethod> methods;

    std::size_t repeats;            // повторов построения (медиана)
    std::size_t queryCount;         // запросов на индексе
    std::size_t linearQueryCount;   // запросов у линейного поиска (он медленный)
    std::size_t quadraticSizeLimit; // предел размера для O(n²)-построения
    unsigned seed;
    int minBirthYear;
    int maxBirthYear;
};

// Настройки по умолчанию: атрибуты «номер» и «фамилия + имя + год
// рождения», размеры 10³–10⁶, все порядки и методы.
ExperimentConfig MakeDefaultExperimentConfig();

// Результат одного сочетания «атрибут, размер, порядок, метод».
struct Measurement
{
    std::string attribute;
    SearchMethod method;
    DataOrder order;
    std::size_t size;
    bool skipped;        // замер пропущен (O(n²) на большом размере)
    double buildMs;      // медиана времени построения, мс
    double findUs;       // среднее время поиска по ключу, мкс
    double rangeUs;      // среднее время поиска по диапазону, мкс
    double averageFound; // среднее число найденных записей на диапазон
    std::size_t keyCount; // различных ключей в индексе (0 у линейного)
};

// Вызывается после каждого замера — для вывода хода эксперимента.
using MeasurementCallback = std::function<void(const Measurement&)>;

// Бросает InvalidArgument при пустых списках или нулевом числе повторов.
ArraySequence<Measurement> RunExperiment(const ExperimentConfig& config,
                                         const MeasurementCallback& onMeasured);

// Сравнение методов config.methods на готовых данных пользователя
// (задание: «сравнение алгоритмов по времени на одних и тех же входных
// данных»). Используются repeats, queryCount, linearQueryCount,
// quadraticSizeLimit и seed из config; остальные поля не нужны.
// Если данные идут по возрастанию ключа, порядок в результате —
// Ascending, иначе AsIs; от этого зависит, пропускается ли O(n²)-построение.
ArraySequence<Measurement> CompareMethodsOnData(const ArraySequence<Person>& people,
                                                const CompositeAttribute& attribute,
                                                const ExperimentConfig& config);

// CSV с заголовком; разделитель ',', числа с точкой.
void WriteMeasurementsCsv(const ArraySequence<Measurement>& measurements, std::ostream& out);

// Короткие латинские обозначения для CSV и графиков.
std::string GetMethodCode(SearchMethod method);
std::string GetOrderCode(DataOrder order);

// Медиана выборки. Бросает InvalidArgument для пустой выборки.
double Median(const ArraySequence<double>& values);
