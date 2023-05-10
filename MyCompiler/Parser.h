#pragma once
#include "Collection.h"
#include "Token.h"
#include <stack>
#include <vector>
#include <memory>
#include <array>
using namespace std;

struct SymbolWithAttr
{
	int symbol;
	shared_ptr<array<int,8>> attr;
	SymbolWithAttr(){}
	SymbolWithAttr(int symbol) {
		this->symbol = symbol;
		if (Grammer::IsTerminal(symbol))
			attr = nullptr;
		else if (Grammer::IsUnterminal(symbol))
			attr = make_shared<array<int,8>>();
	}
	SymbolWithAttr(const SymbolWithAttr&& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
	}
	SymbolWithAttr& operator=(const SymbolWithAttr& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
		return *this;
	}
	SymbolWithAttr(const SymbolWithAttr& sw) {
		symbol = sw.symbol;
		attr = sw.attr;
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
	SymbolWithAttr top() {
		return array[topIndex];
	}
	SymbolWithAttr get(int index) {
		if (index >= 0) return array[index];
		else return array[topIndex - index];
	}
};

class Parser {
private:
	int nonInt;
	//用于分析操作
	int curStatus = 0;
	int action;
	int tokenIndex = 0;
	int curInputToken;
	SymbolStack symbolStack;
	stack <int> statusStack;

	vector <string> errorInfo;
	Collection* collectionPtr;
	vector <Token>* tokenStream = nullptr;
	Grammer* grammer;

	Parser();
	Parser(const Parser& p) {}
	~Parser() {}
	void operator=(const Parser& p) {}
	string makeErrorInfo(Token);
	bool RedressNon();
	void shift(int);
	int Reduce(int productionIndex);
	void PopToken(int cnt);

	static Parser* parserPtr;
public:
	static Parser* ParserFactory();
	void SetInput(vector <Token>& tokenStream);
	bool Analyse();
	void PrintError();
};