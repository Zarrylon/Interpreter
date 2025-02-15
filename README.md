# Interpreter
## Description:
Implemented a basic interpretetor with lexical (lexer) and synxtax (parser) analyses of a custom programming language SIGNAL.

Lexer:
* Classifies keywords, delimiters, constants and Identifiers. Stores their positions (row, column) and assigns unique IDs.
* Build tables with the stored information.
* Ignores whitespaces, new lines, tabulations and comments.
* Detects and handles errors.

Parser:
* Reads and processes tables from Lexer.
* Generates an abstract syntax tree.
* Detects and handles errors.

## SIGNAL grammar:
![Grammar](doc/grammar.png)

## Lexer state diagram:
![Lexer](doc/lex_diagram.png)
