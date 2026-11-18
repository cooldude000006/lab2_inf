#!/usr/bin/env bash
# Проверка памяти ЛР-2 (DESIGN.md, нефункциональное требование 6).
# Запуск из корня проекта в WSL/Linux:  bash docs/memcheck/run_memcheck.sh
# Логи сохраняются рядом со скриптом.
set -u
cd "$(dirname "$0")/../.."
LOGS=docs/memcheck

cmake --preset debug >/dev/null && cmake --build --preset debug >/dev/null
cmake --preset asan >/dev/null && cmake --build --preset asan >/dev/null

# 1. Санитайзеры (ASan + UBSan): тесты и программа на сценарии ввода.
./build/asan/tests/lab2_tests > "$LOGS/sanitizers_tests.log" 2>&1
echo "sanitizers tests: exit $?"
./build/asan/lab2 < "$LOGS/console_session.txt" > "$LOGS/sanitizers_console.log" 2>&1
echo "sanitizers console: exit $?"

# 2. Valgrind на обычной отладочной сборке.
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
    ./build/debug/tests/lab2_tests > /dev/null 2> "$LOGS/valgrind_tests.log"
echo "valgrind tests: exit $?"
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
    ./build/debug/lab2 < "$LOGS/console_session.txt" > /dev/null 2> "$LOGS/valgrind_console.log"
echo "valgrind console: exit $?"
