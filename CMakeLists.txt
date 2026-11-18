cmake_minimum_required(VERSION 3.20)

project(Lab2
        VERSION 1.0
        DESCRIPTION "Data indexing: binary search over a sorted sequence vs AVL tree"
        LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Общие настройки
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif ()

option(LAB2_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(LAB2_BUILD_TESTS "Build unit tests" ON)
option(LAB2_BUILD_BENCH "Build benchmarks" ON)

# Флаги предупреждений (DESIGN.md, нефункциональное требование 5) задаются
# через интерфейсную цель, чтобы не распространять их на чужой код (GoogleTest).
add_library(lab2_warnings INTERFACE)
target_compile_options(lab2_warnings INTERFACE -Wall -Wextra -pedantic)

# Санитайзеры (нефункциональное требование 6) включаются для всех целей.
# Работают в сборке GCC под Linux/WSL; MinGW их не поддерживает.
if (LAB2_SANITIZE)
    message(STATUS "Lab2: sanitizers enabled (address, undefined)")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=address,undefined)
endif ()

# ---------------------------------------------------------------------------
# Ядро: библиотека структур данных и прикладная логика.
# Шаблоны лежат целиком в заголовках (.h + .tpp); компилируется только
# нешаблонная прикладная логика.
# ---------------------------------------------------------------------------
add_library(lab2_core STATIC
        src/Attribute.cpp
        src/AttributeValue.cpp
        src/Benchmark.cpp
        src/CompositeKey.cpp
        src/Database.cpp
        src/Index.cpp
        src/LinearSearch.cpp
        src/Person.cpp
        src/PersonCsv.cpp
        src/PersonGenerator.cpp
        src/SearchMethod.cpp
        src/TextUtils.cpp)
target_include_directories(lab2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lab2_core PUBLIC lab2_warnings)

# ---------------------------------------------------------------------------
# Консольная программа
# ---------------------------------------------------------------------------
# Консольный интерфейс — отдельная библиотека, чтобы его можно было
# проверить автотестом со сценарием ввода.
add_library(lab2_ui STATIC
        src/ConsoleIO.cpp
        src/ConsoleUI.cpp
        src/ConsoleUISearch.cpp)
target_include_directories(lab2_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lab2_ui PUBLIC lab2_core)

add_executable(lab2
        src/main.cpp)
target_link_libraries(lab2 PRIVATE lab2_ui)

# ---------------------------------------------------------------------------
# Тесты и замеры
# ---------------------------------------------------------------------------
if (LAB2_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (LAB2_BUILD_BENCH)
    add_subdirectory(bench)
endif ()
