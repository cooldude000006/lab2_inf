#pragma once

// Человек — запись коллекции (DESIGN.md, п. 2.1 «Данные», п. 4.5).
//
// Хранит номер, фамилию, имя, отчество и год рождения; полное имя,
// «Фамилия И.О.» и возраст на заданный год вычисляются.
// Методы чтения GetFirstName и т. п. заданы примером класса Person
// в задании (п. 2.5).
//
// Ограничения на поля (нарушение → InvalidArgument):
// - номер положительный;
// - фамилия и имя не пустые; отчество может быть пустым;
// - в именах нет символа ';' и переводов строки (это разделители CSV);
// - год рождения от kMinBirthYear до текущего года.

#include <string>

class Person
{
public:
    static constexpr int kMinBirthYear = 1900;

    // Пустая запись. Нужна только контейнерам (ArraySequence создаёт
    // массив через new T[]); в коллекцию людей такие записи не попадают.
    Person();

    Person(int id, const std::string& lastName, const std::string& firstName,
           const std::string& middleName, int birthYear);

    int GetId() const;
    const std::string& GetLastName() const;
    const std::string& GetFirstName() const;
    const std::string& GetMiddleName() const;
    int GetBirthYear() const;

    // «Иванов Пётр Сергеевич»; без отчества — «Иванов Пётр».
    std::string GetFullName() const;

    // «Иванов П.С.»; без отчества — «Иванов П.».
    std::string GetFio() const;

    // Полных лет на год year (без учёта дня рождения: year − год рождения).
    // Бросает InvalidArgument, если year раньше года рождения.
    int GetAge(int year) const;

private:
    int id_;
    std::string lastName_;
    std::string firstName_;
    std::string middleName_;
    int birthYear_;
};

// Записи равны, если равны все поля.
bool operator==(const Person& left, const Person& right);
bool operator!=(const Person& left, const Person& right);

// Текущий год по системным часам.
int CurrentYear();
