#pragma once

// Вспомогательные функции для работы с текстом (ввод пользователя, CSV).
// Строки во всей программе хранятся в кодировке UTF-8.

#include <cstddef>
#include <string>

// Убирает пробелы, табуляции и переводы строки по краям.
std::string Trim(const std::string& text);

// Разбирает целое число без лишних символов (пробелы по краям допустимы).
// Бросает InvalidArgument с понятным сообщением, если text не целое число
// или не помещается в int. what — что именно разбирается («год рождения»).
int ParseInteger(const std::string& text, const std::string& what);

// Первый символ строки UTF-8 (один или несколько байтов).
// Пустая строка → пустая строка.
std::string FirstUtf8Character(const std::string& text);

// Число символов (а не байтов) в строке UTF-8.
std::size_t Utf8Length(const std::string& text);

// Дополняет строку пробелами до width символов справа или слева — для
// таблиц в консоли. std::setw считает байты и сбивает столбцы
// с русскими буквами. Длинная строка возвращается без изменений.
std::string PadRight(const std::string& text, std::size_t width);
std::string PadLeft(const std::string& text, std::size_t width);

// Число с фиксированным числом знаков после точки.
std::string FormatFixed(double value, int digits);
