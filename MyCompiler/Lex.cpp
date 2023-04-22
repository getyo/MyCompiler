#include "Lex.h"
#include <fstream>
#include "FileManager.h"
#include "Token.h"

extern FileManager fileManager;
extern SymbolTable symbolTable;;
unordered_map <string, Ty_TokenKind> Lexeme::tokenKindStr2Num;
vector<string> Lexeme::tokenKindNum2Str;

void Lexeme::FollowPos(SyntalNodePtr& node, Ty_FollowPos& followPos) {
	if (!node->leftChild && !node->rightChild) return;
	FollowPos(node->leftChild, followPos);
	if (node->GetName() == STAR_NODE) {
		for (auto& i : node->leftChild->lastPos) {
			followPos[i].insert(node->leftChild->firstPos.begin(),
				node->leftChild->firstPos.end());
		}
		return;
	}
	FollowPos(node->rightChild, followPos);
	if (node->GetName() == CAT_NODE) {
		for (auto& i : node->leftChild->lastPos) {
			followPos[i].insert(node->rightChild->firstPos.begin(),
				node->rightChild->firstPos.end());
		}
	}
}

void Lexeme::InputReg() {
	int tokenKindNum = 0;
	while (regIn.good()) {
		string s;
		getline(regIn, s);
		string tokenKindName = s.substr(0, s.find_first_of(":"));
		string reg = s.substr(s.find_first_of(":") + 1, s.size());
		//暂且不填写map中tokenKindName对应的tokenKind
		//实际这里的tokenKind和对于正则表达式语法树的TreeId保持一致
		tokenKindStr2Num.insert({ tokenKindName,tokenKindNum++ });
		tokenKindNum2Str.push_back(tokenKindName);
		regArray.push_back(reg);
	}

}

void Lexeme::ConstructFollowPosTable() {
	for (auto& reg : regArray) {
		auto treePtr = SyntalTree::ConstructSyntalTree(reg);
		treeArray.push_back(treePtr);
		followPosTable.push_back(Ty_FollowPos());
		auto followPos = (--followPosTable.end());
		//确保followPos中有足够个集合保存对应Pos的follow集
		followPos->insert(followPos->begin(), treePtr->GetMaxPos() + 1, set<int>());
		auto root = treePtr->GetRoot();
		//SyntalTree::PrintTree(treePtr);
		FollowPos(root, *followPos);
	}
}

void Lexeme::Tree2Dfa() {
	int treeCnt = treeArray.size();
	for (size_t i = 0; i < treeCnt; i++)
	{
		dfaVec.push_back(DFA(treeArray[i], followPosTable[i], i));
		//dfaVec[i].Print();
	}
}

void Lexeme::DfaVec2Nfa() {
	nfaPtr = make_shared<NFA>(dfaVec);
}

void Lexeme::Nfa2Dfa() {
	unoptimizedDfa = FiniteAutomata::Nfa2Dfa(*nfaPtr);
}

void Lexeme::OutputDfa(DFA& dfa) {
	regOut << dfa.Info();
	regOut.close();
}

void Lexeme::InitLex() {
	InputReg();
	ConstructFollowPosTable();
	Tree2Dfa();
	/*
	cout << "\n";
	for (auto& i : dfaVec) {
		i.Print();
		cout << "\n";
	}
	*/
	DfaVec2Nfa();
	//cout << "\n";
	//nfaPtr->Print();
	Nfa2Dfa();
	//cout << "\n";
	//unoptimizedDfa.Print();
	//cout << "\n";
	OutputDfa(unoptimizedDfa);
	/*测试：ReadDFA
	ifstream in(regOutDir + "\\" + "dfa.txt");
	if (in.is_open()) {
		DFA d = DFA::ReadDfa(in);
		d.Print();
		in.close();
	}
	*/
}

string JumpBlank(string& s, int& ptr) {
	while (s[ptr] == ' ' || s[ptr] == '\t')ptr++;
	return s.substr(ptr, s.size());
}

string nextWord(string s) {
	int findBlank = s.find_first_of(' ');
	int findTab = s.find_first_of('\t');
	if (findBlank == -1 && findTab == -1)
		return s;
	else if (findBlank == -1 && findTab != -1)
		return s.substr(0, findTab);
	else if (findBlank != -1 && findTab == -1)
		return s.substr(0, findBlank);
	else {
		int end = min(findBlank, findTab);
		return s.substr(0, end);
	}
}

vector<Token> Lexeme::Analyse() const {
	vector<Token> tokenVec;
	if (istream* in = dynamic_cast<istream*>(input)) {
		string line;
		int row = 1, JumpedCharCnt = 0;
		int col = 1;
		while (in->good()) {
			getline(*in, line);
			string word = line;
			while (word.size() > 0) {
				word = JumpBlank(word, JumpedCharCnt);
				col += JumpedCharCnt;
				JumpedCharCnt = 0;
				//只有空格，不进行识别
				if (word.size() == 0) break;

				//识别成功时，JumpedCharCnt保存识别到的Token长度
				Ty_TokenKind tokenKind = unoptimizedDfa.Recognize(word, JumpedCharCnt);
				string lexeme = word.substr(0, JumpedCharCnt);
				if (tokenKind != Token::FAILED) {
					Token token;
					token.kind = tokenKind;
					token.symbolTableIndex = symbolTable.Size();
					symbolTable.Push(TokenAttribute(lexeme, row, col + 1, -1));
					tokenVec.push_back(token);
					//跳过已识别的token
					word = word.substr(JumpedCharCnt, word.size());
				}
				else {
					//未识别成功，跳过当前第一个不为“ ”的子串
					int blankSub = word.find_first_of(' ');
					int tabSub = word.find_first_of('\t');
					string str;
					//如果word中已经没有空格，说明跳过了word的全部内容
					if (blankSub == -1 && tabSub == -1) {
						str = word;
						word = "";
						JumpedCharCnt = str.size();
					}
					else {
						int end;
						if (blankSub != -1) end = blankSub;
						else if (tabSub != -1) end = tabSub;
						else end = min(blankSub, tabSub);
						str = word.substr(0, end);
						word = word.substr(end, word.size());
						JumpedCharCnt = blankSub;
					}
					cerr << "Lexeme recognize error: row " << row << \
						" col " << col << " lexeme : " << str << "\n";
				}
				col += JumpedCharCnt;
				JumpedCharCnt = 0;
			}
			++row;
			col = 0;
		}
	}
	return tokenVec;
}

Lexeme::Lexeme() {
	this->input = nullptr;
	if (!fileManager.IsDir(regInDir)) {
		fileManager.CreateDir(regInDir);
	}
	regIn.open(regInDir + "\\reg.txt");
	if (!regIn.is_open())
		cerr << "No input file :" << regInDir + "\\reg.txt";
	if (!fileManager.CreateDir(regOutDir)) {
		fileManager.CreateDir(regOutDir);
	}
	regOut.open(regOutDir + "\\dfa.txt");
}

int main() {
	Lexeme lex;
	lex.InitLex();
	lex.SetInput(cin);
	auto tokenVec = lex.Analyse();
	for (auto token : tokenVec) {
		token.Print();
		symbolTable[token.symbolTableIndex].Print();
		cout << "\n";
	}
}
