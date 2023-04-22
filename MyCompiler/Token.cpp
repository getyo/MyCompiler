#include "Token.h"
#include "Lex.h"

const Ty_TokenKind Token::FAILED = 555555;

void Token::Print() {
	cout << "TokenKind:" << Lexeme::tokenKindNum2Str[kind]<< \
		"	symbolTableIndex: " << symbolTableIndex << "	";
}

void TokenAttribute::Print() {
	cout << "Lexeme: " << lexeme << "	row: " << row\
		<< "	col: " << col << "	typeID:	" << typeID;
}

TokenAttribute SymbolTable::operator[](int i) {
	return table[i];
}

void SymbolTable::Push(TokenAttribute ta) {
	table.push_back(ta);
}

size_t SymbolTable::Size() {
	return table.size();
}
