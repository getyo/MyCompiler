#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Parser.h"
#include "Triple.h"
#include "Debug.h"

SymbolTable symbolTable;
FileManager* fileManager;
Generator* genPtr;
CodeStore cs;

int main() {

	fileManager = FileManager::FileManagerFactory();

	Lexeme* lex = Lexeme::LexemeFactory();
	lex->InitLex();
	Parser* parser = Parser::ParserFactory();

	lex->SetInput(cin);
	auto tokenStream = lex->Analyse();
	lex->PrintError();

	genPtr = Generator::GeneratorFactory(cs);

	parser->SetInput(tokenStream);
	bool syntalPass = parser->Analyse();
	if (!syntalPass) parser->PrintError();

#ifdef _ICP
	cout << endl << '\n';
	Generator::Print();
#endif // _ICP

}