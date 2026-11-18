// Точка входа консольной программы ЛР-2 (DESIGN.md, п. 4.10).

#include <iostream>

#include "ConsoleIO.h"
#include "ConsoleUI.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

namespace
{

// В консоли Windows по умолчанию не UTF-8, и русский текст превращается
// в «кракозябры». Возвращает true, если ввод идёт из настоящей консоли
// Windows: тогда ConsoleIO читает её напрямую в UTF-16 (std::cin там не
// умеет читать русские буквы). В Linux/WSL и при вводе из файла или
// окна запуска CLion читается обычный std::cin.
bool SetUpConsole()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    DWORD mode = 0;
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
#else
    return false;
#endif
}

} // namespace

int main()
{
    const bool useSystemConsole = SetUpConsole();
    ConsoleIO io(std::cin, std::cout, useSystemConsole);
    ConsoleUI ui(io);
    ui.Run();
    return 0;
}
