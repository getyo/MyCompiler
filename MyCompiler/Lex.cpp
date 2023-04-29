#include "Lex.h"
#include <fstream>
#include "FileManager.h"
#include "Token.h"

Lexeme* Lexeme::lexemePtr = nullptr;
extern FileManager* fileManager;
extern SymbolTable symbolTable;;
unordered_map <string, Ty_TokenKind> Lexeme::tokenKindStr2Num;
vector<string> Lexeme::tokenKindNum2Str;

Lexeme::~Lexeme(){
	if (lexemePtr != nullptr) delete lexemePtr;
}

Lexeme* Lexeme::LexemeFactory() {
	if (lexemePtr == nullptr)
		lexemePtr = new Lexeme();
	return lexemePtr;
}

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
		if (s == "") continue;
		//注释行
		else if (s[0] == '/' && s[1] == '/') continue;
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

int curPos;

vector<Token> Lexeme::Analyse() const {
	vector<Token> tokenVec;
	if (istream* in = dynamic_cast<istream*>(input)) {
		string line;
		int row = 0;
		bool errorFlag = false;
		while (in->good()) {
			++row;
			curPos = 0;
			getline(*in, line);
			while (curPos < line.size()) {
				int prePos = curPos;
				Ty_TokenKind tokenKind = unoptimizedDfa.Recognize(line, curPos);
				if (tokenKind != Token::FAILED) {
					Token token;
					token.kind = tokenKind;
					token.symbolTableIndex = symbolTable.Size();
					string lexeme = line.substr(prePos, curPos - prePos);
					symbolTable.Push(TokenAttribute(lexeme, row, prePos + 1, -1));
					tokenVec.push_back(token);
					if (errorFlag) errorFlag = false;
				}
				else {
					//第一次出现错误，打印信息，否则跳过当前符号继续分析
					if (!errorFlag) {
						cout << "Error: line " << row \
							<< " col " << prePos+1 << " " << "Unidentified Symbol:"\
							<< line.substr(prePos,curPos-prePos) << '\n';
						errorFlag = true;
					}
					++curPos;
				}
			}
		}
	}
	return tokenVec;
}

Lexeme::Lexeme() {
	this->input = nullptr;
	if (!fileManager->IsDir(regInDir)) {
		fileManager->CreateDir(regInDir);
	}
	regIn.open(regInDir + "\\reg.txt");
	if (!regIn.is_open())
		cerr << "No input file :" << regInDir + "\\reg.txt";
	if (!fileManager->CreateDir(regOutDir)) {
		fileManager->CreateDir(regOutDir);
	}
	regOut.open(regOutDir + "\\dfa.txt");
}

/*
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
**/
