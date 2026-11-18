#include "ConsoleIO.h"

#include "Exceptions.h"
#include "TextUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{

#ifdef _WIN32
// Читает строку из консоли Windows в UTF-16 и переводит в UTF-8.
// Возвращает false, если ввод закончился.
bool ReadWindowsConsoleLine(std::string& line)
{
    const HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    const DWORD bufferSize = 1024;
    wchar_t buffer[bufferSize];
    std::wstring wide;
    while (true)
    {
        DWORD read = 0;
        if (!ReadConsoleW(input, buffer, bufferSize, &read, nullptr) || read == 0)
        {
            return false;
        }
        wide.append(buffer, read);
        if (wide.back() == L'\n')
        {
            break;
        }
    }
    // Ctrl+Z в начале строки — конец ввода, как у std::cin.
    if (!wide.empty() && wide[0] == L'\x1A')
    {
        return false;
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
                                         nullptr, 0, nullptr, nullptr);
    line.assign(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), &line[0], size,
                        nullptr, nullptr);
    return true;
}
#endif

ArraySequence<std::string> SplitList(const std::string& text)
{
    ArraySequence<std::string> parts;
    std::string current;
    for (char symbol : text)
    {
        if (symbol == ' ' || symbol == ',' || symbol == '\t' || symbol == ';')
        {
            if (!current.empty())
            {
                parts.Append(current);
                current.clear();
            }
        }
        else
        {
            current += symbol;
        }
    }
    if (!current.empty())
    {
        parts.Append(current);
    }
    return parts;
}

} // namespace

const char* InputEnded::what() const noexcept
{
    return "ввод завершён";
}

ConsoleIO::ConsoleIO(std::istream& in, std::ostream& out, bool useSystemConsole)
    : in_(in), out_(out), useSystemConsole_(useSystemConsole)
{
}

std::ostream& ConsoleIO::Out()
{
    return out_;
}

std::string ConsoleIO::ReadLine(const std::string& prompt)
{
    out_ << prompt;
    out_.flush();
    std::string line;
    if (!ReadRawLine(line))
    {
        out_ << "\n";
        throw InputEnded();
    }
    return Trim(line);
}

std::string ConsoleIO::ReadNonEmptyLine(const std::string& prompt)
{
    while (true)
    {
        const std::string line = ReadLine(prompt);
        if (!line.empty())
        {
            return line;
        }
        PrintHint("Пустой ввод: введите значение.");
    }
}

int ConsoleIO::ReadInt(const std::string& prompt, int minimum, int maximum)
{
    while (true)
    {
        try
        {
            const int value = ParseInteger(ReadLine(prompt), "число");
            if (value >= minimum && value <= maximum)
            {
                return value;
            }
            PrintHint("Введите число от " + std::to_string(minimum) + " до "
                      + std::to_string(maximum) + ".");
        }
        catch (const InvalidArgument& error)
        {
            PrintHint(error.what());
        }
    }
}

int ConsoleIO::ReadIntOrDefault(const std::string& prompt, int defaultValue, int minimum,
                                int maximum)
{
    const std::string fullPrompt = prompt + " [" + std::to_string(defaultValue) + "]: ";
    while (true)
    {
        const std::string line = ReadLine(fullPrompt);
        if (line.empty())
        {
            return defaultValue;
        }
        try
        {
            const int value = ParseInteger(line, "число");
            if (value >= minimum && value <= maximum)
            {
                return value;
            }
            PrintHint("Введите число от " + std::to_string(minimum) + " до "
                      + std::to_string(maximum) + " или нажмите Enter для значения по умолчанию.");
        }
        catch (const InvalidArgument& error)
        {
            PrintHint(error.what());
        }
    }
}

bool ConsoleIO::ReadYesNo(const std::string& prompt, bool defaultValue)
{
    const std::string fullPrompt = prompt + (defaultValue ? " [Д/н]: " : " [д/Н]: ");
    while (true)
    {
        const std::string line = ReadLine(fullPrompt);
        if (line.empty())
        {
            return defaultValue;
        }
        if (line == "д" || line == "Д" || line == "y" || line == "Y" || line == "да")
        {
            return true;
        }
        if (line == "н" || line == "Н" || line == "n" || line == "N" || line == "нет")
        {
            return false;
        }
        PrintHint("Ответьте «д» (да) или «н» (нет).");
    }
}

ArraySequence<int> ConsoleIO::ReadIntList(const std::string& prompt, int minimum, int maximum)
{
    while (true)
    {
        const ArraySequence<std::string> parts = SplitList(ReadLine(prompt));
        if (parts.GetIsEmpty())
        {
            PrintHint("Введите хотя бы одно число.");
            continue;
        }
        try
        {
            ArraySequence<int> values;
            for (std::size_t i = 0; i < parts.GetLength(); ++i)
            {
                const int value = ParseInteger(parts.Get(i), "число");
                if (value < minimum || value > maximum)
                {
                    throw InvalidArgument("число " + parts.Get(i) + " вне пределов от "
                                          + std::to_string(minimum) + " до "
                                          + std::to_string(maximum));
                }
                values.Append(value);
            }
            return values;
        }
        catch (const InvalidArgument& error)
        {
            PrintHint(std::string(error.what()) + ". Введите числа через пробел.");
        }
    }
}

int ConsoleIO::ReadMenuChoice(const std::string& title, const ArraySequence<std::string>& items,
                              const std::string& backLabel)
{
    out_ << "\n=== " << title << " ===\n";
    for (std::size_t i = 0; i < items.GetLength(); ++i)
    {
        out_ << "  " << (i + 1) << ". " << items.Get(i) << "\n";
    }
    out_ << "  0. " << backLabel << "\n";
    return ReadInt("Выберите пункт: ", 0, static_cast<int>(items.GetLength()));
}

bool ConsoleIO::ReadRawLine(std::string& line)
{
#ifdef _WIN32
    if (useSystemConsole_)
    {
        return ReadWindowsConsoleLine(line);
    }
#endif
    return static_cast<bool>(std::getline(in_, line));
}

void ConsoleIO::PrintHint(const std::string& hint)
{
    out_ << "  ! " << hint << "\n";
}
