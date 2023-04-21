#pragma once
#include <string>
using namespace std;
typedef unsigned int Ty_TokenKind;
typedef unsigned int Ty_TypeID;
struct Token
{
	Ty_TokenKind kind;
	int symbolTableIndex;
};

struct TokenAttribute {
	string lexeme;
	int coord[2];
	Ty_TypeID typeID;
};

class SymbolTable{
	
};