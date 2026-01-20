cmake_minimum_required(VERSION 3.14)


# ===============================================
# Compiler Configuration
# ===============================================

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(eqsat-mod-ac)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address,undefined")
    message(STATUS "Building in DEBUG mode with flags: ${CMAKE_CXX_FLAGS}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -march=native -flto -g")
    message(STATUS "Building in RELEASE mode with flags: ${CMAKE_CXX_FLAGS}")
else()
    message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
endif()

# ===============================================
# External Dependencies
# ===============================================

# Small vector and unordered_dense (local submodules)
add_subdirectory(external/small_vector)
add_subdirectory(external/unordered_dense)

# Fetch Catch2 for testing
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

# Fetch argparse for command-line tools
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v3.1
)
FetchContent_MakeAvailable(argparse)

# ===============================================
# Equality Saturation Library
# ===============================================

set(LIB_SOURCES
    src/theory.cpp
    src/symbol_table.cpp
    src/union_find.cpp
    src/query.cpp
    src/compiler.cpp
    src/egraph.cpp
    src/handle.cpp
    src/utils/permutation.cpp
    src/engine.cpp
    src/database.cpp
    src/parser.cpp
    src/relations/row_store.cpp
    src/relations/relation_ac.cpp
    src/indices/trie_index.cpp
    src/indices/multiset_index.cpp
    src/sets/sorted_vec_set.cpp
    src/sets/abstract_set.cpp
)

add_library(eqsat STATIC ${LIB_SOURCES})

target_include_directories(eqsat PUBLIC
    src
    external/small_vector/source/include
    external/unordered_dense/include
)

target_link_libraries(eqsat PUBLIC
    gch::small_vector
    unordered_dense::unordered_dense
)

# ===============================================
# Unit Tests
# ===============================================

add_executable(unittests
    tests/unit/test_sorted_set.cpp
    tests/unit/test_abstract_set.cpp
    tests/unit/test_set_implementations.cpp
    tests/unit/test_compiler.cpp
    tests/unit/test_union_find.cpp
    tests/unit/test_trie_index.cpp
    tests/unit/test_permutation.cpp
    tests/unit/test_engine.cpp
    tests/unit/test_parser.cpp
    tests/unit/test_multiset.cpp
    tests/unit/test_multiset_index.cpp
    tests/unit/test_ephemeral_ids.cpp
    tests/unit/test_query_builder.cpp
)

target_include_directories(unittests PRIVATE src tests/utils)
target_link_libraries(unittests PRIVATE eqsat Catch2::Catch2WithMain)

# ===============================================
# System Tests
# ===============================================

add_executable(systemtests
    tests/system/test_congruence.cpp
    # tests/system/test_database.cpp
    tests/system/test_egraph_basic.cpp
    tests/system/test_egraph_rewrite.cpp
    tests/system/test_multiplicative_identity.cpp
    tests/system/test_ac_operators.cpp
    tests/system/test_ac_advanced.cpp
)

target_include_directories(systemtests PRIVATE src tests/utils)
target_link_libraries(systemtests PRIVATE eqsat Catch2::Catch2WithMain)

# ===============================================
# Test Configuration
# ===============================================

enable_testing()
add_test(NAME unit_tests COMMAND unittests)
add_test(NAME system_tests COMMAND systemtests)

# ===============================================
# Demo
# ===============================================

# add_executable(groups evaluation/groups.cpp)
# target_include_directories(groups PRIVATE src)
# target_link_libraries(groups PRIVATE eqsat)
# target_link_libraries(groups PRIVATE argparse)

# add_executable(horner evaluation/horner.cpp)
# target_include_directories(horner PRIVATE src)
# target_link_libraries(horner PRIVATE eqsat)

# add_executable(endomorphism evaluation/endomorphism.cpp)
# target_include_directories(endomorphism PRIVATE src)
# target_link_libraries(endomorphism PRIVATE eqsat)

# add_executable(ainva evaluation/a_inv_a.cpp)
# target_include_directories(ainva PRIVATE src)
# target_link_libraries(ainva PRIVATE eqsat)
