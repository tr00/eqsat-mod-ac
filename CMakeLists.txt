cmake_minimum_required(VERSION 3.14)
project(eqsat-mod-ac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/theory.cpp
    src/symbol_table.cpp
    src/sorted_set.cpp
    src/set_interface.cpp
    src/query.cpp
    src/pattern_compiler.cpp
)

# Create main executable
add_executable(main ${SOURCES})

# Include directories
target_include_directories(main PRIVATE src)

# Find or fetch Catch2
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# Test sources (exclude main.cpp)
set(TEST_SOURCES
    src/theory.cpp
    src/symbol_table.cpp
    src/sorted_set.cpp
    src/set_interface.cpp
    src/query.cpp
    src/pattern_compiler.cpp
)

# Create unified test executable
add_executable(tests
    test/test_sorted_set.cpp
    test/test_set_interface.cpp
    test/test_pattern_compiler.cpp
    ${TEST_SOURCES}
)

target_include_directories(tests PRIVATE src)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# Enable testing
enable_testing()
add_test(NAME all_tests COMMAND tests)

# Add custom target for easy testing
add_custom_target(runtest
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
