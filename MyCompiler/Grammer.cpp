#include "Grammer.h"
#include <fstream>
#include "Debug.h"
#include "FileManager.h"
#include "Lex.h"
#include "Triple.h"

int Grammer::END_OF_GRAMMER;
string curSymbol;
string curLine;
int curLineIt;
int row;
int Grammer::maxTerminal;
int Grammer::maxUnterminal;
int Grammer::startSymbol;
vector <Production> Grammer::productions;
vector<int> Grammer::funParaCnt;
unordered_map<string, int> Grammer::funStr2Int;
vector<string> Grammer::funInt2Str;
Grammer* Grammer::grammerPtr = nullptr;
vector<string> Grammer::grammerSymbolNum2Str;
unordered_map<string, int> Grammer::grammerSymbolStr2Num;
unordered_map <int, array<string, 8>> Grammer::attrMap;

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
		return false;
	else return true;
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
static string RowAndCol() {
	string s = " row : " + to_string(row) + " column : " + to_string(curLineIt + 1) + " ";
	return s;
}
static bool IsDigit(char c) {
	if ((c >= '0' && c <= '9') || (c == '-')) return true;
	else return false;
}

void Grammer::ReadAttrMap() {
	bool isInh = false;
	string smbStr = NextSymbol();
	if (smbStr == "-") {
		smbStr = NextSymbol();
		isInh = true;
	}
	if (!Grammer::grammerSymbolStr2Num.count(smbStr)) {
		cerr << "Grammer : Unrecognized Unterminal : " << smbStr << " \t" << RowAndCol();
		abort();
	}
	int smbID = Grammer::GetSymbolID(smbStr);
	if (isInh) smbID = -smbID;
	string temp = NextSymbol();
	if (temp != ":") {
		cerr << "Grammer : Incorrect Input : " << " \t" << RowAndCol();
		abort();
	}
	string attrIndex, attrStr;
	attrIndex = NextSymbol();
	attrStr = NextSymbol();
	while (true) {
		if (!IsDigit(attrIndex[0])) {
			cerr << "Grammer : Incorrect Input : should Input digit" << " \t" << RowAndCol();
		}
		attrMap[smbID][stoi(attrIndex)] = attrStr;
		attrIndex = NextSymbol();
		if (attrIndex.empty()) return;
		attrStr = NextSymbol();
	}
	
}

void Grammer::ReadAction(Production &p,ActionList &actionList) {
	actionList.push_back(make_shared<Action>());
	auto action = actionList[actionList.size() - 1];
	string s;
	while (true) {
		s = NextSymbol();
		if (s == "##") 
			break;
		//处理函数调用
		if (funStr2Int.count(s)) { 
#ifdef DEBUG
			int paraCnt = 0;
#endif // DEBUG
			action->requested.push_back(Action::FUN);
			action->requested.push_back(Grammer::GetFunID(s));
			++curLineIt;
			if (curLine[curLineIt] != '(') {
				cerr << "\nGrammer: " + RowAndCol() + " Incorrect Input: Auxiliary Function should have ()\n";
				abort();
			}
			++curLineIt;
			while (true) {
				if (curLine[curLineIt] == ')') { ++curLineIt;  break; };
				switch (curLine[curLineIt])
				{
				case ' ': {++curLineIt; break; }
				case ',': {++curLineIt; break; }
				case '$': {
					action->requested.push_back(Action::DIGIT);
					++curLineIt;
					string num = "";
					while (IsDigit(curLine[curLineIt])) num += curLine[curLineIt++];
					action->requested.push_back(stoi(num));
#ifdef DEBUG
					paraCnt += 1;
#endif // DEBUG
					break;
				}
				case '\'': {
					action->requested.push_back(Action::OP);
					string opStr = ""; 
					++curLineIt;
					while (curLine[curLineIt] != '\'') {
						opStr += curLine[curLineIt];
						++curLineIt;
					}
					int op = Generator::GetIcopInt(opStr);
					action->requested.push_back(op);
					++curLineIt;
#ifdef DEBUG
					paraCnt += 1;
#endif // DEBUG
					break;
				}
				default: {
					if (!IsDigit(curLine[curLineIt])) {
						cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should input digit\n";
						abort();
					}
					string d = "";
					while (IsDigit(curLine[curLineIt])) d += curLine[curLineIt++];
					action->requested.push_back( stoi(d) );

					if (curLine[curLineIt] != '.') {
						cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should follow dot after digit\n";
						abort();
					}
					else ++curLineIt;

					if (!IsDigit(curLine[curLineIt])) {
						cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should input digit\n";
						abort();
					}
					d = "";
					while (IsDigit(curLine[curLineIt])) d += curLine[curLineIt++];
					action->requested.push_back( stoi(d) );
#ifdef DEBUG
					++paraCnt;
#endif // DEBUG
					   break;
				}
				}
			}
#ifdef DEBUG
			int realCnt = Grammer::GetFunParaCnt(s);
			ASSERT(paraCnt == realCnt,"Grammer: Function "\
				+ s + " must have " + to_string(realCnt) + " parameters ");
#endif // DEBUG

		}
		else if (s.size() == 1) {
			switch (s[0])
			{
			case '+': {action->requested.push_back(Action::ADD); break; }
			case '-': {action->requested.push_back(Action::MINUS); break; }
			case '*': {action->requested.push_back(Action::MULT); break; }
			case '/': {action->requested.push_back(Action::DIV); break; }
			case '%': {action->requested.push_back(Action::REM); break; }
			case '=': {action->requested.push_back(Action::ASSIGN); break; }
			case ';': { actionList.push_back(make_shared<Action>());
						action = actionList[actionList.size() - 1];	
						break;
			}
			default:
				break;
			}
		}
		else if (s[0] == '$') {
			action->requested.push_back(Action::DIGIT);
			int i = 1;
			string num = "";
			while (IsDigit(s[i])) num += s[i++];
			action->requested.push_back(stoi(num));
			continue;
		}
		//处理符号.属性
		else {
			if (!IsDigit(s[0])) {
				cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should input digit\n";
				abort();
			}
			int i = 0;
			string d = "";
			while (IsDigit(s[i])) d += s[i++];
				action->requested.push_back(stoi(d));

			if (s[i] != '.') {
				cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should follow dot after digit\n";
				abort();
			}
			else ++i;
				
			if (!IsDigit(s[i])) {
				cerr << "\nGrammer:" + RowAndCol() + "Incorrect Input: should input digit\n";
				abort();
			}
			d = "";
			while (IsDigit(s[i])) d += s[i++];
			action->requested.push_back(stoi(d));
		}
	}
}

void Grammer::ReadFunction(ifstream& in) {
	++row;
	getline(in, curLine);
	string funName,temp,paraCnt;
	int funID = 0;
	while (curLine != "END$") {
		if ((curLine[0] == '/' && curLine[1] == '/') || curLine.size() == 0) {
			getline(in, curLine);
			curLineIt = 0;
			++row;
			continue;
		}
		funName = NextSymbol();
		funStr2Int.insert({ funName,funID });
		funInt2Str.push_back(funName);
		++funID;
		temp = NextSymbol();
		if (temp != ":") {
			cerr << "\nGrammer: " + RowAndCol() + " Incorrect Input: Auxiliary Function should have parameter count\n";
			abort();
		}
		paraCnt = NextSymbol();
		funParaCnt.push_back(stoi(paraCnt));
		getline(in, curLine);
		curLineIt = 0;
		++row;
	}
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
	row = 0;
	ReadFunction(in);
	while (in.good()) {
		getline(in, curLine);
		++row;
		curLineIt = 0;
		int dotPos = -1;
		if (!curLine.size() || (curLine[0] == '/' && curLine[1] == '/')) continue;
		Production* p = new Production();
		while (curLineIt < curLine.size()) {
			grammerSymbol = NextSymbol();
			if (grammerSymbol == "->" || grammerSymbol == "") continue;
			else if (grammerSymbol == "##") {
				p->actionLists.insert({ dotPos,ActionList() });
				ReadAction(*p,p->actionLists[dotPos]);
				continue;
			}
			else if (grammerSymbol == "$$") {
				ReadAttrMap();
				goto LineEnd;
			}
			//如果没有语法符号的话
			if (!grammerSymbolStr2Num.count(grammerSymbol)) {
				grammerSymbolStr2Num.insert({ grammerSymbol,grammerSymbolNum2Str.size() });
				grammerSymbolNum2Str.push_back(grammerSymbol);
			}
			p->PushBack(grammerSymbolStr2Num[grammerSymbol]);
			++dotPos;
		}
		productions.push_back(*p);
		p = new Production();
	LineEnd:
		continue;
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
		cout << "\n\n";
	}
	cout << "Unterminal: ";
	for (int i = maxTerminal + 1; i <= maxUnterminal; i++)
		cout << Grammer::grammerSymbolNum2Str[i] << " ";
	cout << "\n";
}