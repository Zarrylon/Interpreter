#pragma once

#include <string>
#include <fstream>
#include <queue>
#include <vector>
#include <array>
#include <unordered_map>
#include <list>

enum class SymbolCategories
{
	Whitespace = 0,
	Constant = 1,
	Identifier = 2,
	SingleDelimeter = 3,
	MultiDelimeterAssembly = 41, // $) 
	MultiDelimeterEqual = 42,	// :=
	Comment = 5, // "(*" "*)", also handles delimeters "(" and "($"
	Error = 6
};

struct Symbol
{
	char value;
	SymbolCategories attr;
};

struct Token
{
	int row = 0;
	int col = 0;
	int id = 0;
	std::string value = "";
};

class Lexer
{
public:
	std::unordered_map<std::string, int> singleDelimiters;
	std::unordered_map<std::string, int> multipleDelimiters;
	std::unordered_map<std::string, int> keywords;
	std::unordered_map<std::string, int> constants;
	std::unordered_map<std::string, int> identifiers;

	std::vector<Token> tokens;
	std::list<std::string> errors;

private:
	std::ifstream inputFile;
	std::ofstream outputFile;

	std::array<SymbolCategories, 255> attributes;
	SymbolCategories category;

	int row;
	int col;

public:
	Lexer();
	Lexer(const std::string&);
	~Lexer();

	//void openFile(const std::string&);
	void startLexicalAnalyzer(const std::string& filename);

	void printLexicalResultsToFile(const std::string&);
	void printLexicalResultsToConsole() const; 

private:
	void initializeTables();
	Symbol gets();

	void handleWhitespace(const Symbol&);
	void getErrors(const std::string&);
	void setConstant(const std::string&);
	void setKeyword(const std::string&);
	void setIdentifier(const std::string&);

	void printTokensToFile();
	void printLexemeToFile(const std::unordered_map<std::string, int>&);

	void printTokensToConsole() const;
	void printLexemeToConsole(const std::unordered_map<std::string, int>&) const;
};
