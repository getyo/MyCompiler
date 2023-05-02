#pragma once
#include "Collection.h"
#include "Token.h"
#include <stack>

class Parser {
private:
	int nonInt;
	//用于分析操作
	int curStatus = 0;
	int action;
	int tokenIndex = 0;
	int curInputToken;
	stack <int> symbolStack;
	stack <int> statusStack;

	vector <string> errorInfo;
	Collection* collectionPtr;
	vector <Token>* tokenStream;
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