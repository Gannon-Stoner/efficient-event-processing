cmake_minimum_required(VERSION 3.23)
project(efficient-event-processing LANGUAGES CXX)

# Global compile settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# External dependencies
include(FetchContent)

# Catch2 for unit tests
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.6.0
)
FetchContent_MakeAvailable(catch2)

# Google Benchmark for performance tests
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.4
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(benchmark)

# Core library target
add_library(algotrading STATIC
        src/core/EventBus.cpp
)

target_include_directories(algotrading
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(algotrading PUBLIC pthread)

# Unit tests
add_executable(unit_tests
        tests/EventBusTests.cpp
)
target_link_libraries(unit_tests
        PRIVATE
        algotrading
        Catch2::Catch2WithMain
)

enable_testing()
add_test(NAME all_unit_tests COMMAND unit_tests)

# Benchmarks
set(BENCH_SRC src/benchmarks/bus_latency.cpp)
add_executable(bus_bench ${BENCH_SRC})
target_link_libraries(bus_bench
        PRIVATE
        algotrading
        benchmark::benchmark
)

# Latency benchmark (compiled with ENABLE_LATENCY flag)
set(LAT_BENCH_SRC src/benchmarks/latency_sampling.cpp)
    add_executable(latency_bench ${LAT_BENCH_SRC})
    target_link_libraries(latency_bench PRIVATE algotrading benchmark::benchmark)
    target_compile_definitions(latency_bench PRIVATE ENABLE_LATENCY)
