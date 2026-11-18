#pragma once

// Консольный интерфейс (DESIGN.md, п. 4.10).
//
// Главное меню:
//   1. Данные — генерация, ручной ввод, загрузка и сохранение CSV, просмотр;
//   2. Индексы — построение (атрибут или составной атрибут + метод),
//      список, просмотр содержимого, удаление;
//   3. Поиск — по ключу и по диапазону через выбранный индекс, с замером
//      времени и автоматической проверкой линейным поиском;
//   4. Сравнение методов на текущих данных;
//   5. Эксперимент на сгенерированных данных с экспортом в CSV.
//
// Любая ошибка (Lab2Error) выводится с подсказкой, и работа продолжается.
// Конец ввода завершает программу.
//
// Реализация разделена на два файла: ConsoleUI.cpp (меню, данные, индексы)
// и ConsoleUISearch.cpp (поиск, сравнение, эксперимент).

#include <cstddef>
#include <string>

#include "ArraySequence.h"
#include "Attribute.h"
#include "Benchmark.h"
#include "CompositeKey.h"
#include "ConsoleIO.h"
#include "Database.h"
#include "Index.h"
#include "PersonGenerator.h"
#include "SearchMethod.h"

class ConsoleUI
{
public:
    explicit ConsoleUI(ConsoleIO& io);

    ConsoleUI(const ConsoleUI&) = delete;
    ConsoleUI& operator=(const ConsoleUI&) = delete;

    // Главный цикл; возвращается после «Выход» или конца ввода.
    void Run();

private:
    // --- Меню (ConsoleUI.cpp) ---------------------------------------------
    bool RunMainMenuOnce();
    void DataMenu();
    void IndexMenu();

    // --- Данные -----------------------------------------------------------
    void GenerateData();
    void AddPersonManually();
    void LoadFromFile();
    void SaveToFile();
    void ShowPeople();

    // --- Индексы ----------------------------------------------------------
    void CreateIndex();
    void ListIndexes();
    void ShowIndexEntries();
    void RemoveIndex();
    bool ConfirmSlowBuild(const CompositeAttribute& attribute, SearchMethod method);

    // --- Поиск, сравнение, эксперимент (ConsoleUISearch.cpp) ---------------
    void SearchMenu();
    void SearchByKey(const Index& index);
    void SearchByRange(const Index& index);
    void ReportSearch(const Index& index, const Index::Positions& found, double elapsedUs,
                      const Index::Positions& expected, double linearUs);
    void CompareMethods();
    void RunExperimentMenu();
    void PrintMeasurements(const ArraySequence<Measurement>& measurements, bool showOrder);
    void OfferCsvExport(const ArraySequence<Measurement>& measurements);

    // --- Выбор и ввод -----------------------------------------------------
    CompositeAttribute ReadCompositeAttribute();
    SearchMethod ReadMethod();
    DataOrder ReadGenerationOrder();
    bool ChooseIndex(std::size_t& number);
    CompositeKey ReadKey(const CompositeAttribute& attribute, const std::string& title);

    // --- Вывод ------------------------------------------------------------
    void PrintStatus();
    void PrintPeopleHeader();
    void PrintPerson(std::size_t position);
    void PrintFound(const Index::Positions& positions);
    std::string DescribeIndex(std::size_t number) const;

    ConsoleIO& io_;
    Database database_;
    unsigned nextSeed_;
};
