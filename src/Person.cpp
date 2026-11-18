#include "Person.h"

#include <ctime>

#include "Exceptions.h"
#include "TextUtils.h"

namespace
{

const int kTmBaseYear = 1900; // struct tm хранит год как смещение от 1900

void CheckName(const std::string& name, const std::string& what, bool mayBeEmpty)
{
    if (!mayBeEmpty && Trim(name).empty())
    {
        throw InvalidArgument(what + ": поле не может быть пустым, введите хотя бы одну букву");
    }
    if (name != Trim(name))
    {
        throw InvalidArgument(what + ": уберите пробелы в начале и в конце");
    }
    if (name.find_first_of(";\r\n") != std::string::npos)
    {
        throw InvalidArgument(what + ": символ ';' и переводы строки недопустимы");
    }
}

} // namespace

Person::Person()
    : id_(0), lastName_(), firstName_(), middleName_(), birthYear_(kMinBirthYear)
{
}

Person::Person(int id, const std::string& lastName, const std::string& firstName,
               const std::string& middleName, int birthYear)
    : id_(id), lastName_(lastName), firstName_(firstName), middleName_(middleName),
      birthYear_(birthYear)
{
    if (id <= 0)
    {
        throw InvalidArgument("Номер записи должен быть положительным, получено "
                              + std::to_string(id));
    }
    CheckName(lastName, "Фамилия", false);
    CheckName(firstName, "Имя", false);
    CheckName(middleName, "Отчество", true);

    // Год вычисляется один раз: генератор создаёт миллионы записей,
    // а обращение к системным часам на каждую из них заметно по времени.
    static const int currentYear = CurrentYear();
    if (birthYear < kMinBirthYear || birthYear > currentYear)
    {
        throw InvalidArgument("Год рождения " + std::to_string(birthYear) + " вне допустимых "
                              "пределов: введите год от " + std::to_string(kMinBirthYear)
                              + " до " + std::to_string(currentYear));
    }
}

int Person::GetId() const
{
    return id_;
}

const std::string& Person::GetLastName() const
{
    return lastName_;
}

const std::string& Person::GetFirstName() const
{
    return firstName_;
}

const std::string& Person::GetMiddleName() const
{
    return middleName_;
}

int Person::GetBirthYear() const
{
    return birthYear_;
}

std::string Person::GetFullName() const
{
    std::string fullName = lastName_ + " " + firstName_;
    if (!middleName_.empty())
    {
        fullName += " " + middleName_;
    }
    return fullName;
}

std::string Person::GetFio() const
{
    std::string fio = lastName_ + " " + FirstUtf8Character(firstName_) + ".";
    if (!middleName_.empty())
    {
        fio += FirstUtf8Character(middleName_) + ".";
    }
    return fio;
}

int Person::GetAge(int year) const
{
    if (year < birthYear_)
    {
        throw InvalidArgument("Нельзя вычислить возраст на " + std::to_string(year)
                              + " год: человек родился в " + std::to_string(birthYear_)
                              + " году; выберите год не раньше года рождения");
    }
    return year - birthYear_;
}

bool operator==(const Person& left, const Person& right)
{
    return left.GetId() == right.GetId() && left.GetLastName() == right.GetLastName()
           && left.GetFirstName() == right.GetFirstName()
           && left.GetMiddleName() == right.GetMiddleName()
           && left.GetBirthYear() == right.GetBirthYear();
}

bool operator!=(const Person& left, const Person& right)
{
    return !(left == right);
}

int CurrentYear()
{
    const std::time_t now = std::time(nullptr);
    std::tm local = *std::localtime(&now);
    return local.tm_year + kTmBaseYear;
}
