#pragma once
#include "Production.h"
#include <set>
#include <string>
#include <unordered_map>
#include <iterator>

class Grammer {
private:
	string path = "input\\grammer.txt";
	static Grammer* grammerPtr;
	//默认id terminal从0开始计数
	int maxTerminal;
	//默认Unterminal从terminal的末尾开始计数
	int maxUnterminal;
	int startSymbol;
	vector <Production> productions;
	Grammer();
	Grammer (const Grammer&) {}
	~Grammer();
	string NextSymbol();
	void operator=(const Grammer&) {}
public:
	static int END_OF_GRAMMER;
	static vector<string> grammerSymbolNum2Str;
	static unordered_map <string,int> grammerSymbolStr2Num;
	
	static Grammer* GrammerFactory();
	bool IsTerminal(string name);
	bool IsTerminal(int val);
	bool IsUnterminal(string name);
	bool IsUnterminal(int val);
	Production& operator[](size_t);
	size_t GrammerSymbolCnt();
	size_t ProductionCnt();
	int StartSymbol();
	string Info();
	void Print();
};