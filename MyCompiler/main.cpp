#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Parser.h"
#include "Triple.h"
#include "Debug.h"
#include "Type.h"

SymbolTable symbolTable;
FileManager* fileManager;
Generator* genPtr;
CodeStore cs;
string testCodeFile = "D:\\proj\\MyCompiler\\MyCompiler\\input\\testCode.txt";

int main() {

	fileManager = FileManager::FileManagerFactory();
	Lexeme* lex = Lexeme::LexemeFactory();
	lex->InitLex();
	Parser* parser = Parser::ParserFactory();
	genPtr = Generator::GeneratorFactory(cs);


	ifstream* sourceCode = new ifstream(testCodeFile);
	if (!sourceCode->is_open()) {
		cerr << "input file cann't open : " << testCodeFile;
	}
		
	lex->SetInput(*sourceCode);
	auto tokenStream = lex->Analyse();
	lex->PrintError();


	parser->SetInput(tokenStream);
	bool syntalPass = parser->Analyse();
	if (!syntalPass) parser->PrintError();

#ifdef _ICP
	cout << endl << '\n';
	Generator::Print();
#endif // _ICP
#ifdef _ENVP
	cout << endl; 
	Environment::PrintAll();
#endif // _ENVP

	Environment::DeleteAll();
	Generator::Release();
	Parser::Release();
	Lexeme::Release();
	FileManager::Release();
}