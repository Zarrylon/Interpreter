#include "lexer.h"

#include <iostream>
#include <algorithm>

Lexer::Lexer() : row(1), col(1), attributes(), category()
{
	initializeTables();
}

Lexer::Lexer(const std::string& filename) : row(1), col(1)
{
	inputFile.open(filename);

	if (!inputFile.is_open())
		std::cout << "Error: Can't open input file " << filename << std::endl;

	initializeTables();
}

Lexer::~Lexer()
{
	if (inputFile.is_open())
		inputFile.close();

	if (outputFile.is_open())
		outputFile.close();
}

void Lexer::initializeTables()
{
	singleDelimiters = {
		{ ";", 59},
		{ ",", 44},
		{ ")", 41},
	};

	multipleDelimiters = {
		{ "($", 301},
		{ "$)", 302},
		{ ":=", 303}
	};

	keywords = {
		{ "PROGRAM", 401 },
		{ "BEGIN", 402 },
		{ "END", 403 },
		{ "GOTO", 404 },
		{ "LINK", 405 },
		{ "IN", 406 },
		{ "OUT", 407 },
		{ "RETURN", 408 },
	};

	constants = {};
	identifiers = {};
	tokens = {};

	for (int i = 0; i < 255; i++)
	{
		if (i == 32 || (i >= 8 && i <= 13))
			attributes[i] = SymbolCategories::Whitespace;
		else if (i >= 48 && i <= 57) // 0...9
			attributes[i] = SymbolCategories::Constant;
		else if ((i >= 65 && i <= 90) || (i >= 97 && i <= 122)) // A...Z || a...z
			attributes[i] = SymbolCategories::Identifier;
		else
			attributes[i] = SymbolCategories::Error;
	}

	attributes[36] = SymbolCategories::MultiDelimeterAssembly; // $
	attributes[40] = SymbolCategories::Comment; // (
	attributes[58] = SymbolCategories::MultiDelimeterEqual; // :


	// ; , )
	int value;
	for (const auto& pair : singleDelimiters)
	{
		value = pair.second;
		attributes[value] = SymbolCategories::SingleDelimeter;
	}
}

void Lexer::startLexicalAnalyzer(const std::string& filename)
{
	inputFile.open(filename);

	if (!inputFile.is_open())
	{
		getErrors("empty file");
		return;
	}

	Symbol s;
	Token t;
	std::string tmp;
	s = gets();

	while (!inputFile.eof())
	{
		switch (s.attr)
		{
		case SymbolCategories::Whitespace:
			while (!inputFile.eof())
			{
				s = gets();
				if (s.attr != SymbolCategories::Whitespace)
					break;
			}
			break;

		case SymbolCategories::Constant:
			t.row = row;
			t.col = col - 1;

			while ((!inputFile.eof()) && s.attr == SymbolCategories::Constant)
			{
				tmp += s.value;
				s = gets();
			}

			if (constants.find(tmp) == constants.end())
				setConstant(tmp);

			t.id = constants[tmp];
			t.value = tmp;

			tokens.push_back(t);

			break;

		case SymbolCategories::Identifier:
			t.row = row;
			t.col = col - 1;

			while ((!inputFile.eof()) &&
				(s.attr == SymbolCategories::Identifier || s.attr == SymbolCategories::Constant))
			{
				tmp += s.value;
				s = gets();
			}

			t.value = tmp;

			if (keywords.find(tmp) != keywords.end())
			{
				t.id = keywords[tmp];
				tokens.push_back(t);
				break;
			}

			if (identifiers.find(tmp) == identifiers.end())
			{
				setIdentifier(tmp);
			}

			t.id = identifiers[tmp];

			tokens.push_back(t);

			break;

		case SymbolCategories::SingleDelimeter:
			t.row = row;
			t.col = col - 1;
			tmp += s.value;

			t.value = tmp;

			t.id = (int)s.value;

			tokens.push_back(t);

			if (!inputFile.eof()) s = gets();

			break;

		case SymbolCategories::MultiDelimeterAssembly:
			t.row = row;
			t.col = col - 1;
			tmp += s.value;

			s = gets();

			if (s.value == ')')
			{
				tmp += s.value;
				t.value = tmp;
				t.id = multipleDelimiters[tmp];

				tokens.push_back(t);
			}
			else
			{
				getErrors("expected \")\" after $");
			}

			if (!inputFile.eof()) s = gets();

			break;

		case SymbolCategories::MultiDelimeterEqual:
			t.row = row;
			t.col = col - 1;
			t.id = (int)s.value;
			tmp += s.value;

			s = gets();

			if (s.value == '=')
			{
				tmp += s.value;
				t.id = multipleDelimiters[tmp];
				t.value = tmp;
				tokens.push_back(t);
			}
			else
			{
				t.value = tmp;
				tokens.push_back(t);
			}

			if (!inputFile.eof()) s = gets();

			break;

		case SymbolCategories::Comment:
			t.row = row;
			t.col = col - 1;
			tmp += s.value;
			t.value = tmp;
			t.id = (int)s.value;
			s = gets();

			if (s.value == '*')
			{
				if (inputFile.eof())
				{
					getErrors("Expected *), but found the end of file");
					break;
				}

				s = gets();
				do
				{
					while (s.value != '*' && !inputFile.eof())
					{
						s = gets();
					}

					if (inputFile.eof())
					{
						getErrors("expected *), but found the end of file");
						break;
					}
					else s = gets();

				} while (s.value != ')');

				if (!inputFile.eof()) s = gets();
			}
			else if (s.value == '$')
			{
				tmp += s.value;
				t.value = tmp;
				t.id = multipleDelimiters[tmp];
				tokens.push_back(t);

				if (!inputFile.eof()) s = gets();

				break;
			}
			else tokens.push_back(t); // (

			break;

		case SymbolCategories::Error:
			std::string err = "illegal character ";
			err += s.value;
			getErrors(err);
			if (!inputFile.eof()) s = gets();

			break;
		}

		tmp = "";
	}

	inputFile.close();
}

void Lexer::printLexicalResultsToFile(const std::string& filename)
{
	outputFile.open(filename);

	if (outputFile.is_open())
	{
		if (!tokens.empty())
		{
			outputFile << "Lexemes:" << std::endl;
			printTokensToFile();
			outputFile << std::endl;
		}

		if (!constants.empty())
		{
			outputFile << "Constants:" << std::endl;
			printLexemeToFile(constants);
			outputFile << std::endl;
		}

		if (!identifiers.empty())
		{
			outputFile << "Identifiers:" << std::endl;
			printLexemeToFile(identifiers);
			outputFile << std::endl;
		}

		for (auto const& i : errors)
		{
			outputFile << i << std::endl;
		}

		std::cout << "Lexer Results were printed in: \"" << filename << "\"" << std::endl;
	}

	outputFile.close();
}

void Lexer::printTokensToFile()
{
	outputFile << "\t" << "Row" << "\t" << "Col" << "\t"
		<< "Code" << "\t" << "Lexem" << std::endl << std::endl;

	for (auto const& i : tokens) {
		outputFile << "\t" << i.row << "\t" << i.col << "\t"
			<< i.id << "\t" << i.value << std::endl;
	}

	outputFile << std::endl;
}

void Lexer::printLexemeToFile(const std::unordered_map<std::string, int>& lexeme)
{
	outputFile << "\t" << "Code" << "\t" << "Lexem" << std::endl << std::endl;

	std::vector<std::pair<std::string, int>> lexemeSorted;

	for (auto i = lexeme.begin(); i != lexeme.end(); i++)
	{
		lexemeSorted.push_back(std::make_pair(i->first, i->second));
	}

	std::sort(lexemeSorted.begin(), lexemeSorted.end(),
		[](const std::pair<std::string, int>& l, const std::pair<std::string, int>& r)
		{
			if (l.second != r.second)
			{
				return l.second < r.second;
			}
			return l.first < r.first;
		});

	for (auto const& i : lexemeSorted) {
		outputFile << "\t" << i.second << "\t" << i.first << std::endl;
	}
}

void Lexer::printLexicalResultsToConsole() const
{
	if (!tokens.empty())
	{
		std::cout << "Lexemes:" << std::endl;
		printTokensToConsole();
		std::cout << std::endl;
	}

	if (!constants.empty())
	{
		std::cout << "Constants:" << std::endl;
		printLexemeToConsole(constants);
		std::cout << std::endl;
	}

	if (!identifiers.empty())
	{
		std::cout << "Identifiers:" << std::endl;
		printLexemeToConsole(identifiers);
		std::cout << std::endl;
	}

	for (auto const& i : errors)
	{
		std::cout << i << std::endl;
	}
}

void Lexer::printTokensToConsole() const
{
	std::cout << "\t" << "Row" << "\t" << "Col" << "\t"
		<< "Code" << "\t" << "Lexem" << std::endl << std::endl;

	for (auto const& i : tokens) {
		std::cout << "\t" << i.row << "\t" << i.col << "\t"
			<< i.id << "\t" << i.value << std::endl;
	}

	std::cout << std::endl;
}

void Lexer::printLexemeToConsole(const std::unordered_map<std::string, int>& lexeme) const
{
	std::cout << "\t" << "Code" << "\t" << "Lexem" << std::endl << std::endl;

	std::vector<std::pair<std::string, int>> lexemeSorted;

	for (auto i = lexeme.begin(); i != lexeme.end(); i++)
	{
		lexemeSorted.push_back(std::make_pair(i->first, i->second));
	}

	std::sort(lexemeSorted.begin(), lexemeSorted.end(),
		[](const std::pair<std::string, int>& l, const std::pair<std::string, int>& r)
		{
			if (l.second != r.second)
			{
				return l.second < r.second;
			}
			return l.first < r.first;
		});

	for (auto const& i : lexemeSorted) {
		std::cout << "\t" << i.second << "\t" << i.first << std::endl;
	}
}

void Lexer::handleWhitespace(const Symbol& s)
{
	if ((int)s.value == 10)
	{
		row++;
		col = 1;
	}
	else if ((int)s.value == 9)
	{
		col += 3;
	}
	else if ((int)s.value == 32)
	{
		col++;
	}
}

Symbol Lexer::gets()
{
	Symbol s;
	inputFile.get(s.value);
	if (!inputFile.eof())
	{
		s.attr = attributes[s.value];
		if (s.attr == SymbolCategories::Whitespace)
			handleWhitespace(s);
		else
			col++;
	}

	return s;
}

void Lexer::getErrors(const std::string& message)
{
	std::string err = "Lexer: Error (line ";
	err += std::to_string(row) + ", column " + std::to_string(col - 1) + "): " + message + "\n";

	errors.push_back(err);
}

void Lexer::setKeyword(const std::string& lex)
{
	size_t keywordId = 401 + keywords.size();
	keywords.insert({ lex, keywordId });
}

void Lexer::setConstant(const std::string& lex)
{
	size_t constantId = 501 + constants.size();
	constants.insert({ lex, constantId });
}

void Lexer::setIdentifier(const std::string& lex)
{
	size_t identifierId = 1001 + identifiers.size();
	identifiers.insert({ lex, identifierId });
}
