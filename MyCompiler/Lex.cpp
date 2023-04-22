#include "Lex.h"
#include <fstream>
#include "FileManager.h"
#include "Token.h"

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
			//暂且不填写map中tokenKindName对应的tokenKind
			//实际这里的tokenKind和对于正则表达式语法树的TreeId保持一致
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
			//实际这里的tokenKind和对于正则表达式语法树的TreeId保持一致
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
		//确保followPos中有足够个集合保存对应Pos的follow集
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

vector<Token> Lexeme::Analyse() const {
	vector<Token> tokenVec;
	if (istream* in = dynamic_cast<istream*>(input)) {
		string line;
		int row = 1,ptr = 0;
		int col = 0;
		while (in->good()) {
			getline(*in, line);
			string word = line;
			while (word.size() > 0) {
				word = JumpBlank(line, ptr);
				col += ptr;
				ptr = 0;
				Ty_TokenKind tokenKind = unoptimizedDfa.Recognize(word, ptr);
				string lexeme = word.substr(0, ptr);
				if (tokenKind != Token::FAILED) {
					Token token;
					token.kind = tokenKind;
					token.symbolTableIndex = symbolTable.Size();
					symbolTable.Push(TokenAttribute(lexeme, row, col + 1, -1));
					tokenVec.push_back(token);
				}
				else {
					cerr << "Lexeme recognize error: row " << row << \
						" col " << col << "lexeme : " << lexeme << "\n";
				}
				word = word.substr(ptr, word.size());
				col += ptr;
				ptr = 0;
			}
			++row;
			col = 0;
		}
	}
	return tokenVec;
}

int main() {
	if (!FileManager::CreateMultDir("Lex\\input")) {
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
