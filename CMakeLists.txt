cmake_minimum_required(VERSION 3.14)
project(eqsat-mod-ac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add library source files
set(LIB_SOURCES
    src/theory.cpp
    src/symbol_table.cpp
    src/union_find.cpp
    src/query.cpp
    src/pattern_compiler.cpp
    src/egraph.cpp
    src/permutation.cpp
    src/engine.cpp
    src/database.cpp
    src/relations/row_store.cpp
    src/indices/trie_index.cpp
    src/sets/sorted_set.cpp
    src/sets/abstract_set.cpp
)

# Create static library
add_library(eqsat STATIC ${LIB_SOURCES})

# Include directories
target_include_directories(eqsat PUBLIC src)

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

# Create unified unit test executable
add_executable(unittests
    unittests/test_sorted_set.cpp
    unittests/test_abstract_set.cpp
    unittests/test_pattern_compiler.cpp
    unittests/test_union_find.cpp
    unittests/test_trie_index.cpp
    unittests/test_permutation.cpp
)

target_include_directories(unittests PRIVATE src)
target_link_libraries(unittests PRIVATE eqsat Catch2::Catch2WithMain)

# Create system test executable
add_executable(systemtests
    systemtests/test_egraph_basic.cpp
    systemtests/test_egraph_rewrite.cpp
    systemtests/test_database.cpp
)

target_include_directories(systemtests PRIVATE src)
target_link_libraries(systemtests PRIVATE eqsat Catch2::Catch2WithMain)

# Enable testing
enable_testing()
add_test(NAME unit_tests COMMAND unittests)
add_test(NAME system_tests COMMAND systemtests)

# Add custom target for easy testing
add_custom_target(runtest
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress
    DEPENDS unittests systemtests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add separate targets for unit and system tests
add_custom_target(rununit
    COMMAND ${CMAKE_CTEST_COMMAND} -R unit_tests --output-on-failure --progress
    DEPENDS unittests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(runsystem
    COMMAND ${CMAKE_CTEST_COMMAND} -R system_tests --output-on-failure --progress
    DEPENDS systemtests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
