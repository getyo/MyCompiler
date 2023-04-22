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
	if (ifstream* in = dynamic_cast<ifstream*>(this->regIn)) {
		int tokenKindNum = 0;
		while (in->good()) {
			string s;
			getline(*in, s);
			string tokenKindName = s.substr(0, s.find_first_of(":"));
			string reg = s.substr(s.find_first_of(":") + 1,s.size());
			//���Ҳ���дmap��tokenKindName��Ӧ��tokenKind
			//ʵ�������tokenKind�Ͷ���������ʽ�﷨����TreeId����һ��
			tokenKindStr2Num.insert({ tokenKindName,tokenKindNum++ });
			tokenKindNum2Str.push_back(tokenKindName);
			regArray.push_back(reg);
		}
	}
	else if (istream* in = dynamic_cast<istream*>(this->regIn)) {
		int tokenKindNum = 0;
		while (in->good()) {
			string s;
			getline(*in, s);
			string tokenKindName = s.substr(0, s.find_first_of(":"));
			string reg = s.substr(s.find_first_of(":") + 1, s.size());
			//ʵ�������tokenKind�Ͷ���������ʽ�﷨����TreeId����һ��
			tokenKindStr2Num.insert({ tokenKindName,tokenKindNum });
			tokenKindNum2Str.push_back(tokenKindName);
			regArray.push_back(reg);
		}
	}
}

void Lexeme::ConstructFollowPosTable() {
	for (auto& reg : regArray) {
		auto treePtr = SyntalTree::ConstructSyntalTree(reg);
		treeArray.push_back(treePtr);
		followPosTable.push_back(Ty_FollowPos());
		auto followPos = (--followPosTable.end());
		//ȷ��followPos�����㹻�����ϱ����ӦPos��follow��
		followPos->insert(followPos->begin(), treePtr->GetMaxPos() + 1, set<int>());
		auto root = treePtr->GetRoot();
		SyntalTree::PrintTree(treePtr);
		FollowPos(root, *followPos);
	}
}

void Lexeme::Tree2Dfa() {
	int treeCnt = treeArray.size();
	for (size_t i = 0; i < treeCnt; i++)
	{
		dfaVec.push_back(DFA(treeArray[i], followPosTable[i],i));
		//dfaVec[i].Print();
	}
}

void Lexeme::DfaVec2Nfa() {
	nfaPtr = make_shared<NFA>(dfaVec);
}

void Lexeme::Nfa2Dfa() {
	unoptimizedDfa = FiniteAutomata::Nfa2Dfa(*nfaPtr);
}

void Lexeme::InitLex() {
	InputReg();
	ConstructFollowPosTable();
	Tree2Dfa();
	cout << "\n";
	for (auto& i : dfaVec) {
		i.Print();
		cout << "\n";
	}
	DfaVec2Nfa();
	cout << "\n";
	nfaPtr->Print();
	Nfa2Dfa();
	cout << "\n";
	unoptimizedDfa.Print();
	cout << "\n";
}

string JumpBlank(string& s,int &ptr) {
	while (s[ptr] == ' ')ptr++;
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
		int row = 1,JumpedCharCnt;
		int col = 1;
		while (in->good()) {
			getline(*in, line);
			string word = line;
			while (word.size() > 0) {
				word = JumpBlank(word, JumpedCharCnt);
				col += JumpedCharCnt;
				JumpedCharCnt = 0;
				Ty_TokenKind tokenKind = unoptimizedDfa.Recognize(word, JumpedCharCnt);
				string lexeme = word.substr(0, JumpedCharCnt);
				if (tokenKind != Token::FAILED) {
					Token token;
					token.kind = tokenKind;
					token.symbolTableIndex = symbolTable.Size();
					symbolTable.Push(TokenAttribute(lexeme, row, col + 1, -1));
					tokenVec.push_back(token);
					word = word.substr(JumpedCharCnt, word.size());
				}
				else {
					int blankSub = word.find_first_of(' ');
					string str;
					if (blankSub == -1) {
						str = word;
						word = "";
					}
					else {
						str = word.substr(0, blankSub);
						word = word.substr(blankSub, word.size());
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

int main() {
	if (!fileManager.IsDir("Lex\\input")) {
		fileManager.CreateDir("Lex\\input");
		cerr << "Directory creat failed \n";
		abort();
	}
	ifstream regIn("Lex\\Input\\reg.txt");
	if (!regIn.is_open()) abort();
	Lexeme lex(regIn);
	lex.InitLex();
	lex.SetInput(cin);
	auto tokenVec = lex.Analyse();
	for (auto token : tokenVec) {
		token.Print();
		symbolTable[token.symbolTableIndex].Print();
		cout << "\n";
	}
}
