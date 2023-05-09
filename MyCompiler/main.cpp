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
	Parser* parser = Parser::ParserFactory();

	lex->SetInput(cin);
	auto tokenStream = lex->Analyse();
	lex->PrintError();

	parser->SetInput(tokenStream);
	bool syntalPass = parser->Analyse();
	if (!syntalPass) parser->PrintError();
}