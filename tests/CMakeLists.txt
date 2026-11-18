include(FetchContent)

# GoogleTest скачивается при первой конфигурации проекта.
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
# Для MSVC: та же библиотека времени выполнения, что и у проекта.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lab2_tests
        TestHelpers.h
        test_array_sequence.cpp
        test_basics.cpp
        test_sorted_sequence.cpp
        test_avl_tree.cpp
        test_dictionaries.cpp
        test_utils.cpp
        test_person.cpp
        test_index.cpp
        test_generator_csv.cpp
        test_benchmark.cpp
        test_console.cpp)
target_link_libraries(lab2_tests PRIVATE lab2_ui GTest::gtest_main)

# Регистрирует каждый TEST как отдельный тест CTest (и в окне тестов CLion).
include(GoogleTest)
gtest_discover_tests(lab2_tests)
