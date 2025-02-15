#include "parser.h"

#include <iostream>

Parser::Parser(const std::string& filename) : 
	par({0, 0, 0, 0}),
	head(std::make_shared<Node>(Node{ "<signal-program>", 0, {} })),
	current(head)
{
	lexer.startLexicalAnalyze(filename);
	lexer.printLexicalResultsToFile(lexer_output_path);
}

Parser::~Parser()
{
	if (outputParser.is_open())
		outputParser.close();
}

void Parser::nextToken()
{
	if (par.index < lexer.tokens.size() && doContinue)
	{
		par.id = lexer.tokens[par.index].id;
		par.row = lexer.tokens[par.index].row;
		par.col = lexer.tokens[par.index].col;
		par.index++;
	}
}

void Parser::addNode(Node* root)
{
	if (doContinue)
	{
		std::shared_ptr<Node> n = std::make_shared<Node> (Node{ findInTable(par.id), par.id, {} });
		root->leaf.push_back(n);
		current = n;
	}	
}

void Parser::addNode(Node* root, std::string value)
{
	if (doContinue)
	{
		std::shared_ptr<Node> n = std::make_shared<Node> (Node{ value, 0, {} });
		root->leaf.push_back(n);
		current = n;
	}
}

void Parser::printTreeToConsole(Node* n, int depth)
{
	for (int i = 0; i < depth; i++)
	{
		outputParser << "|  ";
	}

	if (n->id != 0)
		outputParser << n->id << " ";
	outputParser << n->value << std::endl;

	for (int i = 0; i < n->leaf.size(); i++)
	{
		printTreeToConsole(n->leaf[i].get(), depth + 1);
	}
}

std::string Parser::findInTable(int lexId)
{
	std::string value = "";
	if (lexId <= 255)
	{
		if (lexId == 40 || lexId == 58)
			value = char(lexId);
		else value = findByKey(lexer.singleDelimiters, lexId);
	}
	else if (lexId >= 301 && lexId <= 400)
		value = findByKey(lexer.multipleDelimiters, lexId);
	else if (lexId <= 500)
		value = findByKey(lexer.keywords, lexId);
	else if (lexId <= 1000)
		value = findByKey(lexer.constants, lexId);
	else
		value = findByKey(lexer.identifiers, lexId);

	return value;
}

std::string Parser::findByKey(std::unordered_map<std::string, int> map, int val)
{
	for (auto const& i : map)
	{
		if (i.second == val)
			return i.first;
	}

	return "";
}

void Parser::startParsing()
{
	outputParser.open(parser_output_path);

	program();
	printTreeToConsole(head.get(), 0);

	for (auto const& i : errorsParser)
	{
		outputParser << i << std::endl;
	}

	outputParser.close();

	std::cout << "Parser Results were printed in: \"" << parser_output_path << "\"" << std::endl;
}

void Parser::program()
{
	addNode(current.get(), "<program>");
	nextToken();
	auto n = current;

	if (par.id == 401) // PROGRAM
	{
		addNode(current.get());
		current = n;

		nextToken();
		proc_identifier();

		nextToken();
		if (par.id == 59) // ;
		{
			current = n;
			addNode(current.get());
			current = n;

			nextToken();
			block();

			nextToken();
			if (par.id == 59) // ;
			{
				current = n;
				addNode(current.get());
			}
			else showError("';'");
		}
		else showError("';'");
	}
	else showError("keyword 'PROGRAM'");
}

void Parser::block()
{
	addNode(current.get(), "<block>");
	auto n = current;

	if (par.id == 402) // BEGIN
	{
		addNode(current.get());
		current = n;

		nextToken();
		statements_list();

		if (par.id == 403) // END
		{
			current = n;
			addNode(current.get());
		}
		else showError("keyword 'END'");
	}
	else showError("keyword 'BEGIN'");
}

void Parser::statements_list()
{
	addNode(current.get(), "<statement-list>");
	auto n = current;

	if (par.id == 403 || !doContinue) // END or Error
	{
		addNode(current.get(), "<empty>");
	}
	else
	{
		statement();

		nextToken();
		current = n;
		statements_list();
	}
}

void Parser::statement()
{
	addNode(current.get(), "<statement>");
	auto n = current;

	if (par.id >= 501 && par.id <= 1000) // Constant
	{
		current = n;
		u_integer();

		nextToken();
		if (par.id == 58) // :
		{
			current = n;
			addNode(current.get());

			current = n;
			nextToken();
			statement();
		}
		else showError("':'");
	}
	else if (par.id >= 1001) // Identifier
	{
		TreeParser tmpIdPrev = par;
		current = n;
		nextToken();

		// <variable-identifier> := <unsigned-integer>
		// or
		// <procedure-identifier><actual-arguments>
		if (par.id == 303) // :=
		{
			current = n;
			par = tmpIdPrev;
			var_identifier();

			nextToken();
			current = n;
			addNode(current.get());

			current = n;
			nextToken();
			u_integer();

			nextToken();
			current = n;
			if (par.id == 59) // ;
			{
				addNode(current.get());
				current = n;
			}
			else showError("';'");
		}
		else
		{
			current = n;
			par = tmpIdPrev;
			proc_identifier();

			nextToken();
			current = n;
			actualArgs();

			current = n;
			if (par.id == 59) // ;
			{
				addNode(current.get());
				current = n;
			}
			else showError("';'");
		}
	}
	else if (par.id == 404) // GOTO
	{
		current = n;
		addNode(current.get());

		current = n;
		nextToken();
		u_integer();

		current = n;
		nextToken();
		if (par.id == 59) // ;
		{
			addNode(current.get());
			current = n;
		}
		else showError("';'");
	}
	else if (par.id == 405) // LINK
	{
		current = n;
		addNode(current.get());

		nextToken();
		current = n;
		var_identifier();

		nextToken();
		current = n;
		if (par.id == 44) // ,
		{
			addNode(current.get());
			current = n;

			nextToken();
			u_integer();

			current = n;
			nextToken();
			if (par.id == 59) // ;
			{
				addNode(current.get());
				current = n;
			}
			else showError("';'");
		}
		else showError("','");
	}
	else if (par.id == 406 || par.id == 407) // IN || OUT
	{
		current = n;
		addNode(current.get());

		nextToken();
		current = n;
		u_integer();

		nextToken();
		current = n;
		if (par.id == 59)
		{
			addNode(current.get());
			current = n;
		}
		else showError("';'");
	}
	else if (par.id == 408) // RETURN
	{
		current = n;
		addNode(current.get());

		current = n;
		nextToken();
		if (par.id == 59)
		{
			addNode(current.get());
			current = n;
		}
		else showError("';'");
	}
	else if (par.id == 59)
	{
		current = n;
		addNode(current.get());
	}
	else if (par.id == 301) // ($
	{
		current = n;
		addNode(current.get());

		nextToken();
		current = n;
		asmIF_identifier();

		nextToken();
		current = n;
		if (par.id == 302) // $)
		{
			addNode(current.get());
			current = n;
		}
		else showError("'$)'");
	}
	else showError("<statement>");
}

void Parser::actualArgs()
{
	addNode(current.get(), "<actual-arguments>");
	auto n = current;

	if (par.id == 40) // (
	{
		current = n;
		addNode(current.get());

		nextToken();
		current = n;
		var_identifier();

		nextToken();
		current = n;
		if (par.id != 41) // )
		{
			actualArgs_list();
			current = n;

			if (par.id == 41) // )
			{
				addNode(current.get());
				current = n;
			}
			else showError("')'");
		}
		else
		{
			addNode(current.get());
			current = n;
		}

		nextToken();
	}
	else if (par.id == 59) // ;
	{
		current = n;
		addNode(current.get(), "<empty>");
	}
	else showError("<actual-arguments>");
}

void Parser::actualArgs_list()
{
	addNode(current.get(), "<actual-arguments-list>");
	auto n = current;

	if (par.id == 44) // ,
	{
		addNode(current.get());
		current = n;

		nextToken();
		var_identifier();

		nextToken();
		current = n;
		if (par.id != 41) // )
		{
			actualArgs_list();
			current = n;
		}
	}
	else
	{
		current = n;
		addNode(current.get(), "<empty>");
	}
}

void Parser::proc_identifier()
{
	addNode(current.get(), "<procedure-identifier>");
	identifier();
}

void Parser::identifier()
{
	addNode(current.get(), "<identifier>");
	auto n = current;

	if (par.id >= 1001)
	{
		current = n;
		addNode(current.get());
	}
	else showError("<identifier>");
}

void Parser::var_identifier()
{
	addNode(current.get(), "<variable-identifier>");
	identifier();
}

void Parser::asmIF_identifier()
{
	addNode(current.get(), "<assembly-insert-file-identifier>");
	identifier();
}

void Parser::u_integer()
{
	addNode(current.get(), "<unsigned-integer>");

	if (par.id >= 501 && par.id <= 1000)
	{
		addNode(current.get());
	}
	else showError("<unsigned-integer>");
}

void Parser::showError(std::string err)
{
	if (doContinue)
	{
		std::string err_tmp = "Parser: Error (Line " + std::to_string(par.row) + ", Column " + std::to_string(par.col) + "): ";
		err_tmp += err + " expected.";

		errorsParser.push_back(err_tmp);
	}
	doContinue = false;
}
