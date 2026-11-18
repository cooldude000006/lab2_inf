#pragma once

// Ввод и вывод консольного интерфейса (DESIGN.md, п. 4.10).
//
// Читает строки из потока и разбирает их, повторяя запрос с подсказкой,
// пока пользователь не введёт допустимое значение: некорректный ввод
// не завершает программу. Работает с любыми потоками, поэтому интерфейс
// можно проверять автотестом со сценарием ввода.
//
// Если ввод закончился (конец файла, Ctrl+Z / Ctrl+D), бросается
// InputEnded: интерфейс по нему корректно завершает работу.
//
// В Windows при чтении из настоящей консоли строки читаются в UTF-16
// и переводятся в UTF-8: std::cin в консоли Windows не умеет читать
// русские буквы в UTF-8.

#include <cstddef>
#include <exception>
#include <istream>
#include <ostream>
#include <string>

#include "ArraySequence.h"

// Ввод закончился. Не наследуется от Lab2Error: это не ошибка данных,
// а сигнал завершить работу.
class InputEnded : public std::exception
{
public:
    const char* what() const noexcept override;
};

class ConsoleIO
{
public:
    // useSystemConsole — читать из консоли Windows напрямую (только для
    // std::cin в настоящей консоли; в остальных случаях читается in).
    ConsoleIO(std::istream& in, std::ostream& out, bool useSystemConsole);

    std::ostream& Out();

    // Строка без пробелов по краям; может быть пустой.
    std::string ReadLine(const std::string& prompt);

    // Непустая строка.
    std::string ReadNonEmptyLine(const std::string& prompt);

    // Целое из [minimum, maximum].
    int ReadInt(const std::string& prompt, int minimum, int maximum);

    // То же, но пустой ввод означает defaultValue (показывается в запросе).
    int ReadIntOrDefault(const std::string& prompt, int defaultValue, int minimum, int maximum);

    // Да/нет; пустой ввод — defaultValue.
    bool ReadYesNo(const std::string& prompt, bool defaultValue);

    // Непустой список целых из [minimum, maximum] через пробел или запятую.
    ArraySequence<int> ReadIntList(const std::string& prompt, int minimum, int maximum);

    // Меню: заголовок, пункты с номерами 1..n и «0 — назад» (backLabel).
    // Возвращает выбранный номер от 0 до n.
    int ReadMenuChoice(const std::string& title, const ArraySequence<std::string>& items,
                       const std::string& backLabel);

private:
    bool ReadRawLine(std::string& line);
    void PrintHint(const std::string& hint);

    std::istream& in_;
    std::ostream& out_;
    bool useSystemConsole_;
};
