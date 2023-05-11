#pragma once
#include <string>
#include <vector>
#include <iostream>

using namespace std;
typedef unsigned int Ty_TokenKind;
typedef int Ty_TypeID;
class SymbolTable;

struct Token
{
	Ty_TokenKind kind;
	int symbolTableIndex;
	static const Ty_TokenKind FAILED;
	void Print();
};

struct TokenAttribute {
	string lexeme;
	int val = -1;
	int row;
	int col;
	Ty_TypeID typeID = -1;
	TokenAttribute(string& lexeme,int row,int col,Ty_TypeID typeID):\
		lexeme(lexeme), row(row), col(col), typeID(typeID) {}
	void Print();
};

class SymbolTable{
private:
	vector<TokenAttribute> table;
public:
	TokenAttribute& operator[](int i);
	void Push(TokenAttribute ta);
	size_t Size();
};