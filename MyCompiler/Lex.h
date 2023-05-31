#pragma once
#include "SyntaxTree.h"
#include "Debug.h"
#include <set>
#include <unordered_map>
#include "FiniteAutomata.h"
#include <fstream>
using namespace std;

typedef vector<set<int>> Ty_FollowPos;
class Lexeme {
private:
	ifstream regIn;
	ofstream regOut;
	ios* input;
	vector	<Ty_FollowPos> followPosTable;
	vector <string> regArray;
	vector <SyntalTreePtr> treeArray;
	vector <DFA> dfaVec;
	shared_ptr <NFA> nfaPtr;
	DFA unoptimizedDfa;
	vector<string> errorInfo;
	vector<string> sourceCode;
	
	string MakeErrorInfo(string symbol, int row, int col) const;
	//将输入的正则表达式整理好，并且构建语法树，DAF
	void FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos);
	void InputReg();
	void ConstructFollowPosTable();
	void Tree2Dfa();
	void DfaVec2Nfa();
	void Nfa2Dfa();
	void OutputDfa(DFA &dfa);

	void GetInput();

	Lexeme();
	Lexeme(const Lexeme&) {}
	Lexeme(const Lexeme&&) {}
	void operator=(const Lexeme&) {}
	~Lexeme() {}
	static Lexeme* lexemePtr;
public:
	static Lexeme* LexemeFactory();
	static unordered_map <string, Ty_TokenKind> tokenKindStr2Num;
	static vector<string> tokenKindNum2Str;
	void InitLex();
	void SetInput(ios& in) { this->input = &in; }
	vector<Token> Analyse();
	void PrintError();

	static void Release() {
		if (lexemePtr != nullptr) {
			delete lexemePtr;
			lexemePtr = nullptr;
		}
	}
};