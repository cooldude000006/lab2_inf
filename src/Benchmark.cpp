#include "Benchmark.h"

#include <chrono>
#include <iomanip>
#include <memory>
#include <random>

#include "CompositeKey.h"
#include "Exceptions.h"
#include "Index.h"
#include "LinearSearch.h"
#include "MergeSort.h"
#include "Pair.h"

namespace
{

using Clock = std::chrono::steady_clock;

// Из выборки такого размера берутся соседние ключи для диапазонов:
// диапазон захватывает в среднем n / kRangeSampleSize записей.
const std::size_t kRangeSampleSize = 1000;

const double kMicrosecondsPerMillisecond = 1000.0;

// Построение дольше этого не повторяется: случайные колебания времени
// на таком фоне ничтожны, а повторы O(n²)-построений заняли бы минуты.
const double kLongBuildMs = 1000.0;

// Сюда складываются размеры результатов, чтобы компилятор не выбросил
// «бесполезные» вызовы поиска при оптимизации.
volatile std::size_t gResultSink = 0;

double ElapsedMs(Clock::time_point start, Clock::time_point stop)
{
    return std::chrono::duration<double, std::milli>(stop - start).count();
}

struct QuerySet
{
    ArraySequence<CompositeKey> keys;
    ArraySequence<Pair<CompositeKey, CompositeKey>> ranges;
};

// Ключи существующих записей (для поиска по ключу) и узкие диапазоны между
// соседними ключами случайной выборки (для поиска по диапазону).
QuerySet MakeQueries(const ArraySequence<Person>& people, const CompositeAttribute& attribute,
                     std::size_t count, unsigned seed)
{
    QuerySet queries;
    if (people.GetIsEmpty())
    {
        return queries;
    }
    std::mt19937 random(seed);
    std::uniform_int_distribution<std::size_t> position(0, people.GetLength() - 1);

    queries.keys.Reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        queries.keys.Append(attribute.MakeKey(people.Get(position(random))));
    }

    const std::size_t sampleSize =
        people.GetLength() < kRangeSampleSize ? people.GetLength() : kRangeSampleSize;
    ArraySequence<CompositeKey> sample;
    sample.Reserve(sampleSize);
    for (std::size_t i = 0; i < sampleSize; ++i)
    {
        sample.Append(attribute.MakeKey(people.Get(position(random))));
    }
    MergeSort(sample);
    for (std::size_t i = 0; i + 1 < sample.GetLength() && queries.ranges.GetLength() < count; ++i)
    {
        queries.ranges.Append(
            Pair<CompositeKey, CompositeKey>{sample.Get(i), sample.Get(i + 1)});
    }
    return queries;
}

double MeasureFindUs(const Index& index, const ArraySequence<CompositeKey>& keys,
                     std::size_t limit)
{
    const std::size_t count = keys.GetLength() < limit ? keys.GetLength() : limit;
    if (count == 0)
    {
        return 0.0;
    }
    const Clock::time_point start = Clock::now();
    for (std::size_t i = 0; i < count; ++i)
    {
        gResultSink = gResultSink + index.Find(keys.Get(i)).GetLength();
    }
    const Clock::time_point stop = Clock::now();
    return ElapsedMs(start, stop) * kMicrosecondsPerMillisecond / static_cast<double>(count);
}

double MeasureRangeUs(const Index& index,
                      const ArraySequence<Pair<CompositeKey, CompositeKey>>& ranges,
                      std::size_t limit, double& averageFound)
{
    const std::size_t count = ranges.GetLength() < limit ? ranges.GetLength() : limit;
    averageFound = 0.0;
    if (count == 0)
    {
        return 0.0;
    }
    std::size_t found = 0;
    const Clock::time_point start = Clock::now();
    for (std::size_t i = 0; i < count; ++i)
    {
        found += index.FindRange(ranges.Get(i).key, ranges.Get(i).value).GetLength();
    }
    const Clock::time_point stop = Clock::now();
    gResultSink = gResultSink + found;
    averageFound = static_cast<double>(found) / static_cast<double>(count);
    return ElapsedMs(start, stop) * kMicrosecondsPerMillisecond / static_cast<double>(count);
}

bool IsQuadraticBuild(SearchMethod method, DataOrder order)
{
    return method == SearchMethod::BinarySearch && order != DataOrder::Ascending;
}

void CheckConfig(const ExperimentConfig& config)
{
    if (config.attributes.GetIsEmpty() || config.sizes.GetIsEmpty()
        || config.orders.GetIsEmpty() || config.methods.GetIsEmpty())
    {
        throw InvalidArgument("Эксперимент: выберите хотя бы один атрибут, размер, порядок "
                              "данных и метод");
    }
    if (config.repeats == 0 || config.queryCount == 0 || config.linearQueryCount == 0)
    {
        throw InvalidArgument("Эксперимент: число повторов и запросов должно быть больше нуля");
    }
}

Measurement MeasureMethod(const ArraySequence<Person>& people,
                          const CompositeAttribute& attribute, SearchMethod method,
                          DataOrder order, const QuerySet& queries,
                          const ExperimentConfig& config)
{
    Measurement measurement{attribute.GetName(), method, order, people.GetLength(), false,
                            0.0, 0.0, 0.0, 0.0, 0};
    if (IsQuadraticBuild(method, order) && people.GetLength() > config.quadraticSizeLimit)
    {
        measurement.skipped = true;
        return measurement;
    }

    ArraySequence<double> buildTimes;
    std::unique_ptr<Index> index;
    for (std::size_t repeat = 0; repeat < config.repeats; ++repeat)
    {
        index.reset(); // разрушение прошлого индекса не входит в замер
        const Clock::time_point start = Clock::now();
        index = std::make_unique<Index>(&people, attribute, method);
        const Clock::time_point stop = Clock::now();
        buildTimes.Append(ElapsedMs(start, stop));
        if (buildTimes.GetLast() > kLongBuildMs)
        {
            break;
        }
    }
    measurement.buildMs = Median(buildTimes);
    measurement.keyCount = index->GetKeyCount();

    const std::size_t limit =
        method == SearchMethod::Linear ? config.linearQueryCount : config.queryCount;
    measurement.findUs = MeasureFindUs(*index, queries.keys, limit);
    measurement.rangeUs = MeasureRangeUs(*index, queries.ranges, limit, measurement.averageFound);
    return measurement;
}

} // namespace

ExperimentConfig MakeDefaultExperimentConfig()
{
    const std::size_t thousand = 1000;
    const std::size_t defaultRepeats = 3;
    const std::size_t defaultQueryCount = 1000;
    const std::size_t defaultLinearQueryCount = 5;
    const std::size_t defaultQuadraticSizeLimit = 100 * thousand;
    const unsigned defaultSeed = 2024;
    const int defaultMinBirthYear = 1950;
    const int defaultMaxBirthYear = 2005;

    ExperimentConfig config{
        ArraySequence<CompositeAttribute>{
            CompositeAttribute(Attribute(AttributeId::Id)),
            CompositeAttribute(ArraySequence<Attribute>{Attribute(AttributeId::LastName),
                                                        Attribute(AttributeId::FirstName),
                                                        Attribute(AttributeId::BirthYear)})},
        ArraySequence<std::size_t>{thousand, 10 * thousand, 100 * thousand,
                                   thousand * thousand},
        GetAllDataOrders(),
        GetAllSearchMethods(),
        defaultRepeats,
        defaultQueryCount,
        defaultLinearQueryCount,
        defaultQuadraticSizeLimit,
        defaultSeed,
        defaultMinBirthYear,
        defaultMaxBirthYear};
    return config;
}

ArraySequence<Measurement> RunExperiment(const ExperimentConfig& config,
                                         const MeasurementCallback& onMeasured)
{
    CheckConfig(config);
    ArraySequence<Measurement> measurements;
    for (std::size_t a = 0; a < config.attributes.GetLength(); ++a)
    {
        const CompositeAttribute& attribute = config.attributes.Get(a);
        for (std::size_t s = 0; s < config.sizes.GetLength(); ++s)
        {
            const std::size_t size = config.sizes.Get(s);
            for (std::size_t o = 0; o < config.orders.GetLength(); ++o)
            {
                const DataOrder order = config.orders.Get(o);
                // Одинаковое начальное значение: при любом порядке это
                // одни и те же люди, разложенные по-разному.
                PersonGenerator generator(config.seed);
                const ArraySequence<Person> people = generator.GenerateOrdered(
                    size, config.minBirthYear, config.maxBirthYear, attribute, order);
                const QuerySet queries =
                    MakeQueries(people, attribute, config.queryCount, config.seed + 1);

                for (std::size_t m = 0; m < config.methods.GetLength(); ++m)
                {
                    const Measurement measurement = MeasureMethod(
                        people, attribute, config.methods.Get(m), order, queries, config);
                    measurements.Append(measurement);
                    if (onMeasured)
                    {
                        onMeasured(measurement);
                    }
                }
            }
        }
    }
    return measurements;
}

ArraySequence<Measurement> CompareMethodsOnData(const ArraySequence<Person>& people,
                                                const CompositeAttribute& attribute,
                                                const ExperimentConfig& config)
{
    if (config.methods.GetIsEmpty())
    {
        throw InvalidArgument("Сравнение: выберите хотя бы один метод");
    }
    if (config.repeats == 0 || config.queryCount == 0 || config.linearQueryCount == 0)
    {
        throw InvalidArgument("Сравнение: число повторов и запросов должно быть больше нуля");
    }
    const DataOrder order =
        IsSortedByKey(people, attribute) ? DataOrder::Ascending : DataOrder::AsIs;
    const QuerySet queries = MakeQueries(people, attribute, config.queryCount, config.seed);

    ArraySequence<Measurement> measurements;
    for (std::size_t m = 0; m < config.methods.GetLength(); ++m)
    {
        measurements.Append(
            MeasureMethod(people, attribute, config.methods.Get(m), order, queries, config));
    }
    return measurements;
}

void WriteMeasurementsCsv(const ArraySequence<Measurement>& measurements, std::ostream& out)
{
    const int precision = 6;
    out << "attribute,method,order,size,skipped,build_ms,find_us,range_us,avg_found,keys\n";
    out << std::fixed << std::setprecision(precision);
    for (std::size_t i = 0; i < measurements.GetLength(); ++i)
    {
        const Measurement& m = measurements.Get(i);
        out << m.attribute << ',' << GetMethodCode(m.method) << ',' << GetOrderCode(m.order)
            << ',' << m.size << ',' << (m.skipped ? 1 : 0) << ',' << m.buildMs << ','
            << m.findUs << ',' << m.rangeUs << ',' << m.averageFound << ',' << m.keyCount
            << '\n';
    }
}

std::string GetMethodCode(SearchMethod method)
{
    switch (method)
    {
        case SearchMethod::Linear:
            return "linear";
        case SearchMethod::BinarySearch:
            return "binary";
        case SearchMethod::Avl:
            return "avl";
    }
    return "?";
}

std::string GetOrderCode(DataOrder order)
{
    switch (order)
    {
        case DataOrder::Ascending:
            return "ascending";
        case DataOrder::Descending:
            return "descending";
        case DataOrder::Random:
            return "random";
        case DataOrder::AsIs:
            return "as_is";
    }
    return "?";
}

double Median(const ArraySequence<double>& values)
{
    if (values.GetIsEmpty())
    {
        throw InvalidArgument("Медиана пустой выборки не определена");
    }
    ArraySequence<double> sorted = values;
    MergeSort(sorted);
    const std::size_t middle = sorted.GetLength() / 2;
    if (sorted.GetLength() % 2 == 1)
    {
        return sorted.Get(middle);
    }
    return (sorted.Get(middle - 1) + sorted.Get(middle)) / 2.0;
}
