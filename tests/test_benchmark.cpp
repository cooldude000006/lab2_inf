// Тесты модуля замеров (DESIGN.md, п. 4.9). Само время не проверяется —
// оно зависит от машины; проверяется состав и корректность результатов.

#include <gtest/gtest.h>
#include <sstream>
#include <string>

#include "Benchmark.h"
#include "Exceptions.h"
#include "TestHelpers.h"

namespace
{

ExperimentConfig MakeSmallConfig()
{
    ExperimentConfig config = MakeDefaultExperimentConfig();
    config.attributes = ArraySequence<CompositeAttribute>{
        CompositeAttribute(Attribute(AttributeId::Id))};
    config.sizes = ArraySequence<std::size_t>{50, 200};
    config.repeats = 2;
    config.queryCount = 20;
    config.linearQueryCount = 3;
    config.quadraticSizeLimit = 100;
    return config;
}

} // namespace

TEST(BenchmarkTest, MedianOddEvenAndEmpty)
{
    EXPECT_DOUBLE_EQ(Median(ArraySequence<double>{3.0, 1.0, 2.0}), 2.0);
    EXPECT_DOUBLE_EQ(Median(ArraySequence<double>{4.0, 1.0, 3.0, 2.0}), 2.5);
    EXPECT_DOUBLE_EQ(Median(ArraySequence<double>{7.0}), 7.0);
    EXPECT_THROW(Median(ArraySequence<double>()), InvalidArgument);
}

TEST(BenchmarkTest, OneMeasurementPerCombination)
{
    const ExperimentConfig config = MakeSmallConfig();
    std::size_t callbacks = 0;
    const ArraySequence<Measurement> measurements =
        RunExperiment(config, [&callbacks](const Measurement&) { ++callbacks; });

    // 1 атрибут × 2 размера × 3 порядка × 3 метода.
    EXPECT_EQ(measurements.GetLength(), 18u);
    EXPECT_EQ(callbacks, 18u);
}

TEST(BenchmarkTest, QuadraticBuildSkippedAboveLimit)
{
    const ArraySequence<Measurement> measurements = RunExperiment(MakeSmallConfig(), nullptr);
    for (std::size_t i = 0; i < measurements.GetLength(); ++i)
    {
        const Measurement& m = measurements.Get(i);
        const bool expectSkipped = m.method == SearchMethod::BinarySearch
                                   && m.order != DataOrder::Ascending && m.size > 100;
        EXPECT_EQ(m.skipped, expectSkipped)
            << GetMethodCode(m.method) << " " << GetOrderCode(m.order) << " " << m.size;
        if (!m.skipped)
        {
            EXPECT_GE(m.buildMs, 0.0);
            EXPECT_GT(m.findUs, 0.0);
            // Номера уникальны: у индекса столько ключей, сколько записей.
            const std::size_t expectedKeys = m.method == SearchMethod::Linear ? 0 : m.size;
            EXPECT_EQ(m.keyCount, expectedKeys);
        }
    }
}

TEST(BenchmarkTest, InvalidConfigThrows)
{
    ExperimentConfig config = MakeSmallConfig();
    config.methods = ArraySequence<SearchMethod>();
    EXPECT_THROW(RunExperiment(config, nullptr), InvalidArgument);

    config = MakeSmallConfig();
    config.repeats = 0;
    EXPECT_THROW(RunExperiment(config, nullptr), InvalidArgument);
}

TEST(BenchmarkTest, CsvHasHeaderAndRow)
{
    ArraySequence<Measurement> measurements{Measurement{
        "номер", SearchMethod::Avl, DataOrder::Random, 1000, false, 1.5, 0.25, 2.0, 3.0, 1000}};
    std::ostringstream out;
    WriteMeasurementsCsv(measurements, out);
    const std::string csv = out.str();
    EXPECT_EQ(csv.find("attribute,method,order,size,skipped,build_ms,find_us,range_us,"
                       "avg_found,keys\n"),
              0u);
    EXPECT_NE(csv.find("номер,avl,random,1000,0,1.500000,0.250000,2.000000,3.000000,1000\n"),
              std::string::npos);
}
