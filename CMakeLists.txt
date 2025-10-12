cmake_minimum_required(VERSION 3.14)

# Prefer clang++-19 if available, otherwise use system default
find_program(CLANG19_C_COMPILER clang-19)
find_program(CLANG19_CXX_COMPILER clang++-19)

if(CLANG19_CXX_COMPILER AND CLANG19_C_COMPILER)
    set(CMAKE_C_COMPILER "${CLANG19_C_COMPILER}")
    set(CMAKE_CXX_COMPILER "${CLANG19_CXX_COMPILER}")
    message(STATUS "Using clang-19 compiler")
else()
    message(STATUS "clang-19 not found, using system default compiler")
endif()

project(eqsat-mod-ac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prefer mold linker if available, otherwise use system default
find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
    message(STATUS "Using mold linker")
else()
    message(STATUS "mold not found, using system default linker")
endif()

# Add library source files
set(LIB_SOURCES
    src/theory.cpp
    src/symbol_table.cpp
    src/union_find.cpp
    src/query.cpp
    src/compiler.cpp
    src/egraph.cpp
    src/permutation.cpp
    src/engine.cpp
    src/database.cpp
    src/parser.cpp
    src/relations/row_store.cpp
    src/indices/trie_index.cpp
    src/indices/multiset_index.cpp
    src/sets/sorted_vec_set.cpp
    src/sets/abstract_set.cpp
)

# Create static library
add_library(eqsat STATIC ${LIB_SOURCES})

add_subdirectory(external/small_vector)
add_subdirectory(external/unordered_dense)

# Include directories
target_include_directories(eqsat PUBLIC src)
target_include_directories(eqsat PUBLIC external/small_vector/source/include)
target_include_directories(eqsat PUBLIC external/unordered_dense/include)

target_link_libraries(eqsat PUBLIC gch::small_vector)
target_link_libraries(eqsat PUBLIC unordered_dense::unordered_dense)

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
    unittests/test_set_implementations.cpp
    unittests/test_compiler.cpp
    unittests/test_union_find.cpp
    unittests/test_trie_index.cpp
    unittests/test_permutation.cpp
    unittests/test_engine.cpp
    unittests/test_parser.cpp
    unittests/test_multiset.cpp
    unittests/test_multiset_index.cpp
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

# Create demo executable
add_executable(demo demo.cpp)
target_include_directories(demo PRIVATE src)
target_link_libraries(demo PRIVATE eqsat)

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
