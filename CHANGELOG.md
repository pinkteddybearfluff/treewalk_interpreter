# Changelog

All notable changes to this project will be documented in this file.

[//]: # (The format is based on [Keep a Changelog]&#40;https://keepachangelog.com/en/1.1.0/&#41;,)

[//]: # (and this project uses a scheme similar to semantic versioning, however it does not strictly adhere to it.)

## [Unreleased]

### Added

- Better support/rendering of error messages for SyntaxError and SemanticError.
- Add REPL for language.
- Add error objects.
- Add more pattern matching.
- Type checking.
- Add bytecode generation and run code via VM running on bytecode.
- Maybe add package manager.

### Changed

### Removed

## [0.1.17] - 2026-07-08

- Added math standard library.
- Added even more core standard library math functions like assert(), panic(), random(), clock(), eprintln().
- Added methods syntax for string and map.
- Added more methods for string and map in standard library.
- Added support for for-range loops for arrays, strings and maps.
- Added support for switch/match case/pattern matching.
- Added structs with methods.
- Added enumerators with values.
- Added `is` expression for enum values.
- Struct and Enumerator constructors are first class objects.
- Added Function parameters with default values
- Added error handling with try/catch/throw

## [0.1.16] - 2026-06-02

### Added

- Integrated standard library into the interpreter.
- Early version of REPL.

### Fixed

- Return statement within functions no longer cause semantic/syntax error due to previous bug.

## [0.1.15] - 2026-06-01

### Added

- Added support for compound assignment.
- Added logical and, or and not.
- Added support for modulo operator.
- Added standard library math and array functions(currently not linked main programs).
- Added check for interpreter version via --version and -v command and language version via \_\_VERSION__ constant.

### Fixed

- User defined functions with first parameter as array or string or map can no longer use the method syntax.

## [0.1.14] - 2026-06-01

### Added

- Added support for closures.
- Added support for nested calling syntax.

## [0.1.13] - 2026-06-01

### Added

- Added README.md.
- Added CHANGELOG.md.
- Added null language constant.
- Functions are now a first class values.

## [0.1.12] - 2026-05-31

### Added

- Better rendering for runtime error messages.
- Added runtime errors for TypeError, ArityError, NameError, RedeclarationError, IndexError, and ZeroDivisionError

## [0.1.11] - 2026-05-28

### Added

- Added break and continue support for for-loops.

### Fixed

- Stray returns, break and continue (break and continue outside of loops and return statements outside of function
  declarations) now give Syntax/Semantic errors.

## [0.1.10] - 2026-05-28

### Added

- Added support for for-loops.
- Added built-in push function for array and size function for array and string.
- Added support for multiline comments.

## [0.1.9] - 2026-05-27

### Added

- Added support for array literals and array values in RuntimeValue.
- Added support for accessing array elements and string characters through indexing.

## [0.1.8] - 2026-05-26

### Added

- Added support for boolean values and boolean literals.
- added break and continue statement(currently can be seen anywhere even outside loop).

## [0.1.7] - 2026-05-26

### Added

- Added binary operations like concatanation and comparison for strings.
- Separated expressions from statement, statements return nothing whereas expressions return RuntimeValue.
- Added support for empty statement.

## [0.1.6] - 2026-05-26

### Added

- Added function declaration for user-defined functions.
- Added return statement for functions.

## [0.1.5] - 2026-05-22

### Added

- Added string literals and string data types.
- Added built-in print function.

### Changes

- Changed evaluation result type from double to user-defined variant Type which has double, string and bool.

## [0.1.4] - 2026-05-22

### Added

- Added declaration of variable via let keyword.
- Added if else and while loop support.
- Added block scope to language.
- Now a variable declared in inner block shadows the same variable declared in outer block.

### Changes

- Changed the input for interpreter from string to file via ifstream.

## [0.1.3] - 2026-05-20

### Added

- Added binary operations: >, >=, <, <=, ==, !=.

## [0.1.2] - 2026-05-18

### Added

- Added support for variables.
- Added assignment to variables.
- Early version of function call, which has inbuilt functions- max, min, abs, avg

## [0.1.1] - 2026-04-22

### Added

- Initial interpreter which can act as fancy calculator
- Built on lexer, parser, ast.
- Operations supported binary +, -, /, * and unary -.

[unreleased]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.1.4]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.1.3]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.1.2]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.1.1]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main
