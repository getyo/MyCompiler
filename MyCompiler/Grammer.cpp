#include "Grammer.h"
#include <fstream>
#include "Debug.h"
#include "FileManager.h"
#include "Lex.h"

int Grammer::END_OF_GRAMMER;
string curSymbol;
string curLine;
int curLineIt;
Grammer* Grammer::grammerPtr = nullptr;
vector<string> Grammer::grammerSymbolNum2Str;
unordered_map<string, int> Grammer::grammerSymbolStr2Num;

size_t Grammer::ProductionCnt() {
	return productions.size();
}

Grammer::~Grammer() {
	if (grammerPtr == nullptr) return;
}

Production& Grammer::operator[](size_t sub) {
	return productions[sub];
}

size_t Grammer::GrammerSymbolCnt() {
	return maxUnterminal + 1;
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

	maxTerminal = grammerSymbolNum2Str.size()-1;
	startSymbol = maxTerminal + 1;
	//存贮当前行的Production
	vector <int> p;
	while (in.good()) {
		getline(in, curLine);
		curLineIt = 0;
		if (!curLine.size() || (curLine[0] == '/' && curLine[1] == '/')) continue;
		while (curLineIt < curLine.size()) {
			grammerSymbol = NextSymbol();
			if (grammerSymbol == "->" || grammerSymbol == "") continue;
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
	maxUnterminal = grammerSymbolNum2Str.size()-1;
}

int Grammer::StartSymbol() {
	return startSymbol;
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
	int i = 0;
	for (auto& p : productions) {
		cout << i++ << " : ";
		p.Print();
		cout << '\n';
	}
	cout << "Unterminal: ";
	for (int i = maxTerminal + 1; i <= maxUnterminal; i++)
		cout << Grammer::grammerSymbolNum2Str[i] << " ";
	cout << "\n";
}