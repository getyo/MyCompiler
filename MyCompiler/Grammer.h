#pragma once
#include "Production.h"
#include <set>
#include <string>
#include <unordered_map>
#include <iterator>
#include <array>

class Grammer {
private:
	string path = "input\\grammer.txt";
	static Grammer* grammerPtr;
	//默认id terminal从0开始计数
	static int maxTerminal;
	//默认Unterminal从terminal的末尾开始计数
	static int maxUnterminal;
	static int startSymbol;
	static vector <Production> productions;
	static vector<int> funParaCnt;
	static unordered_map<string, int> funStr2Int;
	static vector<string> funInt2Str;
	static vector<string> grammerSymbolNum2Str;
	static unordered_map <string, int> grammerSymbolStr2Num;
	static unordered_map <int, array<string, 8>> attrMap;
	Grammer();
	Grammer(const Grammer&) {}
	~Grammer();
	void ReadAttrMap();
	void ReadFunction(ifstream& in);
	void ReadAction(Production& p);
	string NextSymbol();
	void operator=(const Grammer&) {}
public:
	static int END_OF_GRAMMER;

	static Grammer* GrammerFactory();
	static string GetAttrStr(int symbolID, int attrIndex) { 
		if (attrIndex == 6) {
			return "CodeStart";
		}
		if (attrIndex == 7) {
			return "CodeEnd";
		}
		return attrMap.at(symbolID).at(attrIndex); 
	}
	static string GetSymbolStr(int symbolID) { return grammerSymbolNum2Str[symbolID]; }
	static int GetSymbolID(string symbolName) { return grammerSymbolStr2Num.at(symbolName); }
	static string GetFunName(int funID) { return funInt2Str[funID]; }
	static int GetFunID(string funName) { return funStr2Int.at(funName); }
	static int GetFunParaCnt(int funID) { return funParaCnt.at(funID); }
	static int GetFunParaCnt(string funName) { return funParaCnt[GetFunID(funName)]; }
	static bool IsTerminal(string name);
	static bool IsTerminal(int val);
	static bool IsUnterminal(string name);
	static bool IsUnterminal(int val);
	Production& operator[](size_t);
	size_t GrammerSymbolCnt();
	size_t ProductionCnt();
	int StartSymbol();
	string Info();
	void Print();
};