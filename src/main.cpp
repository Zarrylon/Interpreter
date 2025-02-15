#include "Lexer/lexer.h"
#include "Parser/parser.h"

#include <iostream>
#include <string>

int main()
{
	std::string path = "../tests/";

	std::cout << "File name: ";
	std::string filename;
	std::getline(std::cin, filename);
	
	Parser par(path + filename);
	par.startParsing();

	return 0;
}
