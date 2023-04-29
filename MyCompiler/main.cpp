#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Grammer.h"
#include "Collection.h"

SymbolTable symbolTable;
FileManager* fileManager;
Grammer* grammer;
Collection* collection;

int main() {
	fileManager = FileManager::FileManagerFactory();
	Lexeme* lex = Lexeme::LexemeFactory();
	lex->InitLex();
//	lex->SetInput(cin);

	grammer = Grammer::GrammerFactory();
	grammer->Print();
	collection = Collection::CollectionFactory(grammer);
	collection->Print();
}