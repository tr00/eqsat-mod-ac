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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -g")

# Prefer mold linker if available
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
    src/permutation.cpp
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
    unittests/test_ephemeral_ids.cpp
)

target_include_directories(unittests PRIVATE src)
target_link_libraries(unittests PRIVATE eqsat Catch2::Catch2WithMain)

# ===============================================
# System Tests
# ===============================================

add_executable(systemtests
    systemtests/test_congruence.cpp
    systemtests/test_egraph_basic.cpp
    systemtests/test_egraph_rewrite.cpp
    systemtests/test_multiplicative_identity.cpp
    systemtests/test_ac_operators.cpp
    systemtests/test_ac_advanced.cpp
)

target_include_directories(systemtests PRIVATE src)
target_link_libraries(systemtests PRIVATE eqsat Catch2::Catch2WithMain)

# ===============================================
# Test Configuration
# ===============================================

enable_testing()
add_test(NAME unit_tests COMMAND unittests)
add_test(NAME system_tests COMMAND systemtests)

add_custom_target(runtest
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --progress
    DEPENDS unittests systemtests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

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

# ===============================================
# Demo
# ===============================================

add_executable(groups evaluation/groups.cpp)
target_include_directories(groups PRIVATE src)
target_link_libraries(groups PRIVATE eqsat)
target_link_libraries(groups PRIVATE argparse)

add_executable(horner evaluation/horner.cpp)
target_include_directories(horner PRIVATE src)
target_link_libraries(horner PRIVATE eqsat)

add_executable(endomorphism evaluation/endomorphism.cpp)
target_include_directories(endomorphism PRIVATE src)
target_link_libraries(endomorphism PRIVATE eqsat)

add_executable(ainva evaluation/a_inv_a.cpp)
target_include_directories(ainva PRIVATE src)
target_link_libraries(ainva PRIVATE eqsat)
