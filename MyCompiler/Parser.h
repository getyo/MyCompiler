#pragma once
#include "Collection.h"
#include "Token.h"
#include <stack>
#include <vector>
#include <memory>
#include <array>
#include "Debug.h"
using namespace std;

#define PH_SYM -1
struct SymbolWithAttr
{
	int symbol;
	int symTableIndex;
	shared_ptr<array<int,8>> attr;
	SymbolWithAttr(){}
	SymbolWithAttr(Token t) {
		symbol = t.kind;
		symTableIndex = t.symbolTableIndex;
		attr = nullptr;
	}
	SymbolWithAttr(int symbol) {
		this->symbol = symbol;
		if (Grammer::IsTerminal(symbol)) {
			attr = nullptr;
		}
		else if (Grammer::IsUnterminal(symbol) || symbol == PH_SYM) {
			attr = make_shared<array<int, 8>>();
			for (auto& i : *attr)
				i = ATTR_NON;
		}
	}
	SymbolWithAttr(const SymbolWithAttr&& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
		symTableIndex = sw.symTableIndex;
	}
	SymbolWithAttr& operator=(const SymbolWithAttr& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
		symTableIndex = sw.symTableIndex;
		return *this;
	}
	SymbolWithAttr (const SymbolWithAttr& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
		symTableIndex = sw.symTableIndex;
	}
};

class SymbolStack {
private:
	vector<SymbolWithAttr> array;
	int topIndex = -1;
public:
	SymbolStack() {}
	void push(SymbolWithAttr& s) {
		if ((int)(array.size() - 1) <= topIndex)array.push_back(s);
		else array[topIndex + 1] = s;
		++topIndex;
	}
	void push(SymbolWithAttr&& s) {
		if ((int)(array.size() - 1) <= topIndex)array.push_back(s);
		else array[topIndex + 1] = s;
		++topIndex;
	}
	bool pop() {
		if (topIndex < 0) {
			cerr << "\nSymbolStack has not element\n";
			abort();
			return false;
		}
		else {
			--topIndex;
			return true;
		}
	}
	SymbolWithAttr& top() {
		return array[topIndex];
	}
	SymbolWithAttr& get(int index) {
		if (index > 0) {
			cerr << "SymbolStack.get can't accpet a index which is bigger than 0\n";
			abort();
		}
		else return array[topIndex + index];
	}
	void Print();
};

class Parser {
private:
	static int row, col;
	int nonInt;
	//用于分析操作
	int curStatus = 0;
	int action;
	int tokenIndex = 0;
	int curInputToken;

	bool hasInh = false;
	SymbolStack symbolStack;
	stack <int> statusStack;
	stack<int> dotPosStack;

	vector <string> errorInfo;
	Collection* collectionPtr;
	vector <Token>* tokenStream = nullptr;
	Grammer* grammer;

	Parser();
	Parser(const Parser& p) {}
	~Parser() {}
	void operator=(const Parser& p) {}
	string makeErrorInfo(Token);

	void PrintAttr(int pItr, SymbolWithAttr& head);
	void* GetAttrPtr(int pItr, int smbIndex, int attrIndex);
	SymbolWithAttr ExecuteAction(int pItr,int dotPos);

	bool RedressNon();
	bool NewProduction(int inputSymbol,int curStatus,int preStatus);
	void shift(SymbolWithAttr &swa);
	void shift(int);
	int Reduce(int productionIndex);
	void PopToken(int cnt);

	static Parser* parserPtr;
public:
	static string RowAndCol();
	static Parser* ParserFactory();
	void SetInput(vector <Token>& tokenStream);
	bool Analyse();
	void PrintError();

	static void Release();
};