#include "TextUtils.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "Exceptions.h"

namespace
{

const char* const kBlankCharacters = " \t\r\n";

// Длина символа UTF-8 по первому байту. Для некорректного байта — 1,
// чтобы не выйти за границу строки.
std::size_t Utf8CharacterLength(unsigned char leadByte)
{
    if ((leadByte & 0x80u) == 0)
    {
        return 1;
    }
    if ((leadByte & 0xE0u) == 0xC0u)
    {
        return 2;
    }
    if ((leadByte & 0xF0u) == 0xE0u)
    {
        return 3;
    }
    if ((leadByte & 0xF8u) == 0xF0u)
    {
        return 4;
    }
    return 1;
}

} // namespace

std::string Trim(const std::string& text)
{
    const std::size_t begin = text.find_first_not_of(kBlankCharacters);
    if (begin == std::string::npos)
    {
        return "";
    }
    const std::size_t end = text.find_last_not_of(kBlankCharacters);
    return text.substr(begin, end - begin + 1);
}

int ParseInteger(const std::string& text, const std::string& what)
{
    const std::string trimmed = Trim(text);
    if (trimmed.empty())
    {
        throw InvalidArgument(what + ": значение не задано, введите целое число");
    }

    const int decimalBase = 10;
    char* end = nullptr;
    errno = 0;
    // long long, а не long: в Windows long такой же ширины, как int,
    // и выход за пределы int иначе не отличить.
    const long long value = std::strtoll(trimmed.c_str(), &end, decimalBase);
    if (*end != '\0')
    {
        throw InvalidArgument(what + ": «" + trimmed + "» не целое число, введите цифры без "
                                     "других символов");
    }
    if (errno == ERANGE || value < INT_MIN || value > INT_MAX)
    {
        throw InvalidArgument(what + ": число «" + trimmed + "» слишком большое по модулю");
    }
    return static_cast<int>(value);
}

std::size_t Utf8Length(const std::string& text)
{
    // Байты вида 10xxxxxx продолжают символ; считаются только первые байты.
    std::size_t length = 0;
    for (char byte : text)
    {
        if ((static_cast<unsigned char>(byte) & 0xC0u) != 0x80u)
        {
            ++length;
        }
    }
    return length;
}

std::string PadRight(const std::string& text, std::size_t width)
{
    const std::size_t length = Utf8Length(text);
    return length >= width ? text : text + std::string(width - length, ' ');
}

std::string PadLeft(const std::string& text, std::size_t width)
{
    const std::size_t length = Utf8Length(text);
    return length >= width ? text : std::string(width - length, ' ') + text;
}

std::string FormatFixed(double value, int digits)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(digits) << value;
    return out.str();
}

std::string FirstUtf8Character(const std::string& text)
{
    if (text.empty())
    {
        return "";
    }
    std::size_t length = Utf8CharacterLength(static_cast<unsigned char>(text[0]));
    if (length > text.size())
    {
        length = text.size();
    }
    return text.substr(0, length);
}
