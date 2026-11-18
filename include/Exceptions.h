#pragma once

// Собственные исключения ЛР-2 (PLAN.md, «Исключения»; Голуб, правила 159–160).
//
// Все исключения наследуются от Lab2Error, поэтому консольный интерфейс
// может перехватить любую ошибку программы одним обработчиком, а тесты —
// проверить конкретный вид ошибки. Текст сообщения формируется в месте
// возбуждения и объясняет, что не так.

#include <stdexcept>

// Общий предок всех ошибок программы.
class Lab2Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

// Индекс вне границ коллекции или обращение к элементу пустой коллекции.
class IndexOutOfRange : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};

// В словаре нет элемента с заданным ключом.
class KeyNotFound : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};

// В словаре уже есть элемент с заданным ключом.
class DuplicateKey : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};

// Некорректный аргумент: перевёрнутый диапазон, нулевой указатель,
// недопустимое значение поля и т. п.
class InvalidArgument : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};

// Файл не удалось открыть, прочитать или записать.
class FileError : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};

// Ошибка в содержимом файла. Сообщение содержит номер строки.
class ParseError : public Lab2Error
{
public:
    using Lab2Error::Lab2Error;
};
