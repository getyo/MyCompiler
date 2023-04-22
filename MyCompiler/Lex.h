#pragma once
#include "SyntaxTree.h"
#include "Debug.h"
#include <set>
#include <map>
#include "FiniteAutomata.h"
#include "Token.h"

using namespace std;
class FiniteAutomata;
class DFA;
class NFA;
typedef vector<set<int>> Ty_FollowPos;
class Lexeme {
private:
	ios* regIn;
	ios* out;
	ios* input;
	vector	<Ty_FollowPos> followPosTable;
	vector <string> regArray;
	vector <SyntalTreePtr> treeArray;
	vector <DFA> dfaVec;
	shared_ptr <NFA> nfaPtr;
	DFA unoptimizedDaf;
	//将输入的正则表达式整理好，并且构建语法树，DAF
	void InitLex();
	void FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos);
	void InputReg();
	void ConstructFollowPosTable();
	void Tree2Dfa();
	void DfaVec2Nfa();
	void Nfa2Dfa();

	Lexeme() {}
public:
	static map <string, Ty_TokenKind> tokenKindStr2Num;
	static vector<string> tokenKindNum2Str;
	static int OUTPUT_TO_SCREEN;
	static int OUTPUT_TO_FILE;
	Lexeme(ios& regIn) {
		this->regIn = &regIn;
		this->input = nullptr;
		this->out = nullptr;
		InitLex();
	}
	void SetInput(ios& in) { this->input = &in; }
	void SetOutput(ios& out) { this->out = &out; }
};

/*
namespace Lexeme {
	class NoInputStreamException : exception {};
	class NoOutputOutputStreamException :exception {};
}
*/