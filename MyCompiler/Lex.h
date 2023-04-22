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
	ios* regIn;
	ofstream out;
	ios* input;
	vector	<Ty_FollowPos> followPosTable;
	vector <string> regArray;
	vector <SyntalTreePtr> treeArray;
	vector <DFA> dfaVec;
	shared_ptr <NFA> nfaPtr;
	DFA unoptimizedDfa;
	//将输入的正则表达式整理好，并且构建语法树，DAF
	void FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos);
	void InputReg();
	void ConstructFollowPosTable();
	void Tree2Dfa();
	void DfaVec2Nfa();
	void Nfa2Dfa();
	void OutputDfa(DFA &dfa);
	void ReadDfa(DFA& dfa);

	Lexeme() {}
public:
	class NoInputStreamException : exception {};
	class NoOutputOutputStreamException :exception {};
	static unordered_map <string, Ty_TokenKind> tokenKindStr2Num;
	static vector<string> tokenKindNum2Str;
	void InitLex();
	Lexeme(ios& regIn) {
		this->regIn = &regIn;
		this->input = nullptr;
		out.open("DFA.txt");
	}
	void SetInput(ios& in) { this->input = &in; }
	vector<Token> Analyse() const;
};