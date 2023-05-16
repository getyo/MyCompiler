#pragma once
#include <string>
#include <vector>
#include <iostream>

using namespace std;
typedef unsigned int Ty_TokenKind;
typedef int Ty_TypeID;
class SymbolTable;

#define ATTR_NON  -1052

struct Token
{
	Ty_TokenKind kind;
	int symbolTableIndex;
	static const Ty_TokenKind FAILED;
	void Print();
};

struct TokenAttribute {
	string lexeme;
	int val;
	int row;
	int col;
	Ty_TypeID typeID ;
	TokenAttribute(string& lexeme,int row,int col):\
		lexeme(lexeme), row(row), col(col) { val = ATTR_NON; typeID = ATTR_NON; }
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