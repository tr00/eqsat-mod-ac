# C++ Style Guide

This document outlines the coding style guidelines I follow when writing C++ code.

## General Principles

- **Consistency**: Follow the established patterns in the codebase
- **Readability**: Code should be self-documenting and easy to understand
- **Performance**: Prefer zero-cost abstractions and efficient implementations
- **Safety**: Use modern C++ features to prevent common errors

## Naming Conventions

### Variables and Functions
- Use `snake_case` for variables, functions, and namespaces
- Use descriptive names that clearly indicate purpose
```cpp
int element_count = 0;
bool insert_element(id_t id);
void process_database_indices();
```

### Types and Classes
- Use `PascalCase` for class names, struct names, and type aliases
```cpp
class SortedSet;
struct DatabaseIndex;
using id_t = uint32_t;
```

### Constants and Enums
- Use `SCREAMING_SNAKE_CASE` for compile-time constants
- Use `PascalCase` for enum class names and `snake_case` for enum values
```cpp
constexpr size_t MAX_CAPACITY = 1000;
enum class OperationType {
    insert,
    remove,
    query
};
```

### Member Variables
- Use `snake_case` for member variables
- No special prefix or suffix for member variables
```cpp
class Example {
private:
    std::vector<id_t> data;
    size_t current_size;
};
```

## File Organization

### Include Order
Always order includes in the following sequence:
1. Standard library headers (`<iostream>`, `<vector>`, etc.)
2. External library headers (`<catch2/catch_test_macros.hpp>`, etc.)
3. Our own project headers (`"sorted_set.h"`, `"id.h"`, etc.)

```cpp
#include <algorithm>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "id.h"
#include "sorted_set.h"
```

### File Termination
- Every file should be terminated with a trailing empty line

### Header Guards
- Use `#pragma once` for header guards (simpler and more portable than traditional guards)

## Code Formatting

### Braces and Indentation
- Use 4 spaces for indentation (no tabs)
- Opening brace on same line for functions, classes, and control structures
```cpp
class Example {
public:
    void function() {
        if (condition) {
            // code here
        }
    }
};
```

### Function Declarations
- Return type on same line as function name
- Parameters on same line unless it becomes too long
```cpp
bool insert(id_t id);
SetInterface intersect(const std::vector<std::reference_wrapper<const SetInterface>>& sets);
```

### Spacing
- Space after control flow keywords: `if (`, `for (`, `while (`
- Space around binary operators: `a + b`, `x == y`
- No space between function name and parentheses: `function()`
- Space after commas in parameter lists: `func(a, b, c)`

## Language Features

### Modern C++ Preferences
- Use `auto` when type is obvious from context or when dealing with complex template types
- Prefer range-based for loops when possible
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers
- Use `const` liberally for immutable data
- Prefer `std::string_view` over `const std::string&` for read-only string parameters

### Function Parameters
- Pass small types (primitives, small structs) by value
- Pass large objects by `const&` for read-only access
- Use `std::move` for transfer of ownership
- Use references for output parameters, pointers only when nullability is needed

### Error Handling
- Prefer exceptions for error conditions that should propagate
- Use return codes (bool, optional) for expected failure cases
- Use assertions for debugging and invariant checking

## Class Design

### Member Initialization
- Use member initializer lists in constructors
- Initialize members in declaration order
```cpp
class Example {
private:
    int value;
    std::string name;
    
public:
    Example(int v, std::string n) : value(v), name(std::move(n)) {}
};
```

### Special Member Functions
- Follow Rule of Zero when possible (use RAII)
- When implementing any of copy constructor, copy assignment, destructor, explicitly consider all five
- Mark single-argument constructors as `explicit` unless implicit conversion is intended

### Const Correctness
- Mark member functions `const` when they don't modify object state
- Use `const` for variables that shouldn't change
- Prefer `const` references for parameters that won't be modified

## Templates and Generic Programming

### Template Parameters
- Use `typename` instead of `class` for template parameters (unless specifically requiring a class)
- Use meaningful names for template parameters
```cpp
template<typename Container>
void process_container(const Container& container);
```

### SFINAE and Concepts (when available)
- Prefer concepts over SFINAE when using C++20
- Use clear, descriptive names for concepts

## Comments and Documentation

### When to Comment
- Explain *why* something is done, not *what* is done (code should be self-explanatory for the "what")
- Document complex algorithms or non-obvious implementation choices
- Add TODO comments for known improvements or missing features

### Comment Style
- Use `//` for single-line comments
- Use `/* */` for multi-line comments when necessary
- Keep comments close to the code they describe

## Performance Guidelines

### Memory Management
- Prefer stack allocation over heap allocation when possible
- Use move semantics to avoid unnecessary copies
- Consider memory layout and cache efficiency for performance-critical code

### Algorithm Choices
- Choose appropriate data structures for the use case
- Prefer standard library algorithms over hand-written loops
- Profile before optimizing, but write reasonably efficient code from the start

## Testing

### Test Structure
- Use descriptive test names that explain what is being tested
- Group related tests in sections
- Test both positive and negative cases
- Test edge cases and boundary conditions

### Test Organization
- One test file per source file when practical
- Use the same naming convention: `test_source_name.cpp` for `source_name.cpp`