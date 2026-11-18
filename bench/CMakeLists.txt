# Замеры времени. Смысл имеют только в сборке Release (пресет release).
add_executable(lab2_bench
        benchmark.cpp)
target_link_libraries(lab2_bench PRIVATE lab2_core)

# Папка для results.csv по умолчанию — bench/results в исходниках проекта,
# куда бы ни собиралась программа.
target_compile_definitions(lab2_bench PRIVATE
        LAB2_RESULTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/results")
