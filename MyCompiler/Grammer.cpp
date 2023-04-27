#include "Grammer.h"
#include <fstream>
#include "Debug.h"
#include "FileManager.h"
#include "Lex.h"

string curSymbol;
string curLine;
int curLineIt;
Grammer* Grammer::grammerPtr = nullptr;
vector<string> Grammer::grammerSymbolNum2Str;
unordered_map<string, int> Grammer::grammerSymbolStr2Num;

Grammer::~Grammer() {
	if (grammerPtr != nullptr) delete grammerPtr;
}

bool Grammer::IsUnterminal(int val) {
	if (val > maxTerminal && val <= maxUnterminal) return true;
	else return false;
}

bool Grammer::IsUnterminal(string name) {
	if (Lexeme::tokenKindStr2Num.count(name) == 1)
		return true;
	else return false;
}

bool Grammer::IsTerminal(int val) {
	if (val >= 0 && val <= maxTerminal) return true;
	else return false;
}

bool Grammer::IsTerminal(string name) {
	return IsTerminal(Lexeme::tokenKindStr2Num[name]);
}

string Grammer::NextSymbol() {
	int begin = 0, end = 0;
	while (curLineIt < curLine.size() && curLine[curLineIt] == ' ' || curLine[curLineIt] == '\t')
		++curLineIt;
	begin = curLineIt;
	while (curLineIt < curLine.size() && curLine[curLineIt] != ' ' && curLine[curLineIt] != '\t')
		++curLineIt;
	end = curLineIt;
	return curLine.substr(begin,end - begin);
}

Grammer::Grammer() {
	ifstream in(path);

	if (!in.is_open()) throw FileNotOpen();
	string grammerSymbol;

	//把TokenKind相关信息（终结符）放入语法符号集合
	grammerSymbolNum2Str.insert(grammerSymbolNum2Str.begin(), \
		Lexeme::tokenKindNum2Str.begin(), Lexeme::tokenKindNum2Str.end());
	grammerSymbolStr2Num.insert(Lexeme::tokenKindStr2Num.begin(), \
		Lexeme::tokenKindStr2Num.end());
	//存贮当前行的Production
	vector <int> p;
	while (in.good()) {
		getline(in, curLine);
		curLineIt = 0;
		while (curLineIt < curLine.size()) {
			grammerSymbol = NextSymbol();
			if (grammerSymbol == "->") continue;
			//如果没有语法符号的话
			if (!grammerSymbolStr2Num.count(grammerSymbol)) {
				grammerSymbolStr2Num.insert({ grammerSymbol,grammerSymbolNum2Str.size() });
				grammerSymbolNum2Str.push_back(grammerSymbol);
			}
			p.push_back(grammerSymbolStr2Num[grammerSymbol]);
		}
		productions.push_back(Production(p));
		p.clear();
	}
}

Grammer* Grammer::GrammerFactory() {
	if (Grammer::grammerPtr == nullptr)
		grammerPtr = new Grammer();
	return grammerPtr;
}

string Grammer::Info() {
	string s = "";
	s += (to_string(maxTerminal) + " ");
	s += (to_string(maxUnterminal) + " ");

	for (auto& i : grammerSymbolNum2Str) s += (i+' ');
	s += '\n';

	for (auto& p : productions) s += p.Info() + '\n';
	return s;
}

void Grammer::Print() {
	for (auto& p : productions) {
		p.Print();
		cout << '\n';
	}
}