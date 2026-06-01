# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project uses a scheme similar to semantic versioning, however it does not strictly adhere to it.

## [Unreleased]

### Added

- Add methods for arrays and string.
- Better support/rendering of error messages for SyntaxError and SemanticError.
- Add REPL for language.
- Add support for including/importing files and modules.
- Add support for switch case.
- Support for logical operators
- Support for compound assignment.
- Support for modulo operator.

### Changed

### Removed

## [0.15.0] - 2026-06-01

### Added

- Added support for compound assignment.
- Added logical and, or and not.
- Added support for modulo operator.
- Added standard library math and array functions(currently not linked main programs).
- Added check for interpreter version via --version and -v command and language version via \_\_VERSION__ constant.

## [0.14.0] - 2026-06-01

### Added

- Added support for closures.
- Added support for repeated calling syntax.

## [0.13.2] - 2026-06-01

### Added

- Added README.md.

## [0.13.1] - 2026-05-31

### Added

- Added CHANGELOG.md.

## [0.13.0] - 2026-05-31

### Added

- Added null language constant.
- Made functions as first class citizens.
- Changed the way environment/scopes was stored for a linked structure.

### Removed

- Removed EnvironmentStack, stack used to store scopes/environment.

## [0.12.0] - 2026-05-31

### Added

- Better rendering for runtime error messages.
- Added runtime errors for TypeError, ArityError, NameError, RedeclarationError, IndexError, and ZeroDivisionError

## [0.11.1] - 2026-05-28

### Added

- Added break and continue support for for-loops.

### Fixed

- Stray returns, break and continue (break and continue outside of loops and return statements outside of function
  declarations) now give Syntax/Semantic errors.

## [0.11.0] - 2026-05-28

### Added

- Added support for for-loops.

## [0.10.0] - 2026-05-28

### Added

- Added built-in push function for array and size function for array and string.
- Added support for multiline comments.

## [0.9.0] - 2026-05-27

### Added

- Added support for array literals and array values in RuntimeValue.
- Added support for accessing array elements and string characters through indexing.

## [0.8.0] - 2026-05-26

### Added

- Added support for boolean values and boolean literals.
- added break and continue statement(currently can be seen anywhere even outside loop).

## [0.7.0] - 2026-05-26

### Added

- Added binary operations like concatanation and comparison for strings.
- Separated expressions from statement, statements return nothing whereas expressions return RuntimeValue.
- Added support for empty statement.

### Changes

- Changed variant Type to variant RuntimeValue.

## [0.6.0] - 2026-05-26

### Added

- Added functionTable to keep data from user defined functions.
- Added function declaration for user-defined functions.
- Added return statement for functions.

### Changes

- The entire program is evaluated all at once, instead of read line then evaluate.

## [0.5.0] - 2026-05-22

### Added

- Added string literals and string data types.
- Added built-in print function.

### Changes

- Changed evaluation result type from double to user-defined variant Type which has double, string and bool.

## [0.4.0] - 2026-05-22

### Added

- Added declaration to function via let.
- Added if else and while loop support.
- Added block scope to language.
- Added a stack of environment in which scopes are pushed to it when entering new block and popped after exiting (
  lexical scoping).
- Now a variable declared in inner block shadows the same variable declared in outer block.

### Changes

- Changed the input for interpreter from string to file via ifstream.

## [0.3.1] - 2026-05-20

### Added

- Added helper utility functions for parser.

## [0.3.0] - 2026-05-19

### Added

- Added binary operations: >, >=, <, <=, ==, !=.
- Better debugging statements for parser and pretty print for ast.

### Removed

- Removed buffer stack from token stream.

## [0.2.0] - 2026-05-18

### Added

- Added support for variables which uses environment/scope to store data.
- Added assignment to variables.
- Early version of function call, which has inbuilt functions- max, min, abs, avg
- Added buffer stack to token stream.

## [0.1.0] - 2026-04-22

### Added

- Initial interpreter which can act as fancy calculator
- Built on lexer, parser, ast.
- ast has nodes which return result in int.
- Operations supported binary +, -, /, * and unary -.

[unreleased]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.0.4]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.0.3]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.0.2]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main

[0.0.1]: https://github.com/pinkteddybearfluff/treewalk_interpreter/tree/main
