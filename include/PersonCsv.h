#pragma once

// Чтение и запись коллекции людей в CSV (DESIGN.md, п. 4.8).
//
// Формат: кодировка UTF-8, разделитель ';', первая строка — заголовок
//   id;last_name;first_name;middle_name;birth_year
// затем по одной записи в строке. Пустые строки пропускаются,
// окончания строк Windows (\r\n) допускаются.
//
// Ошибки:
// - файл не открывается или не записывается → FileError;
// - неверное число полей, нечисловой номер или год, недопустимое значение
//   поля → ParseError с номером строки; при ошибке ничего не загружается.

#include <istream>
#include <ostream>
#include <string>

#include "ArraySequence.h"
#include "Person.h"

void WritePeopleCsv(const ArraySequence<Person>& people, std::ostream& out);
ArraySequence<Person> ReadPeopleCsv(std::istream& in);

// Путь в UTF-8 (в том числе с русскими буквами).
void SavePeopleToFile(const ArraySequence<Person>& people, const std::string& path);
ArraySequence<Person> LoadPeopleFromFile(const std::string& path);
