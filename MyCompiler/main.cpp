#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Grammer.h"

SymbolTable symbolTable;
FileManager* fileManager;
Grammer* grammer;

int main() {
	fileManager = FileManager::FileManagerFactory();
	Lexeme* lex = new Lexeme();
	lex->InitLex();
	lex->SetInput(cin);
	lex->Analyse();
	grammer = Grammer::GrammerFactory();
	grammer->Print();
}