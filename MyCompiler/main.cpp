#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Parser.h"

SymbolTable symbolTable;
FileManager* fileManager;

int main() {
	fileManager = FileManager::FileManagerFactory();
	Lexeme* lex = Lexeme::LexemeFactory();
	lex->InitLex();
	lex->SetInput(cin);
	Parser* parser = Parser::ParserFactory();
	auto tokenStream = lex->Analyse();
	parser->SetInput(tokenStream);
	bool syntalPass = parser->Analyse();
	if (!syntalPass) parser->PrintError();
	delete parser;
}