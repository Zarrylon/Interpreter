#pragma once

#include "../Lexer/lexer.h"

#include <fstream>
#include <vector>
#include <list>

struct Node
{
	std::string value = "";
	int id = -1;
	std::vector<std::shared_ptr<Node>> leaf;
};

class Parser
{
private:
	struct TreeParser
	{
		int index = 0;
		int id = 0;
		int row = 0;
		int col = 0;
	};

	TreeParser par;

	Lexer lexer;

	std::shared_ptr<Node> head;
	std::shared_ptr<Node> current;

	std::list<std::string> errorsParser;

	std::ofstream outputParser;

	std::string lexer_output_path = "../tests/outputLex.txt";
	std::string parser_output_path = "../tests/outputPar.txt";

	bool doContinue = true;

public:
	Parser(const std::string&);
	~Parser();

	void startParsing();

private:
	void nextToken();
	std::string findInTable(int);
	std::string findByKey(std::unordered_map<std::string, int>, int);

	void showError(std::string);

	void addNode(Node*);
	void addNode(Node*, std::string);
	void printTreeToConsole(Node*, int);

	void program();
	void block();
	void statements_list();
	void statement();
	void actualArgs();
	void actualArgs_list();
	void var_identifier();
	void proc_identifier();
	void asmIF_identifier();
	void identifier();
	void u_integer();
};
