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
	string regInDir = "Lex\\input";
	string regOutDir = "Lex\\output";
	ifstream regIn;
	ofstream regOut;
	ios* input;
	vector	<Ty_FollowPos> followPosTable;
	vector <string> regArray;
	vector <SyntalTreePtr> treeArray;
	vector <DFA> dfaVec;
	shared_ptr <NFA> nfaPtr;
	DFA unoptimizedDfa;
	//�������������ʽ����ã����ҹ����﷨����DAF
	void FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos);
	void InputReg();
	void ConstructFollowPosTable();
	void Tree2Dfa();
	void DfaVec2Nfa();
	void Nfa2Dfa();
	void OutputDfa(DFA &dfa);
	void ReadDfa(DFA& dfa);

public:
	class NoInputStreamException : exception {};
	class NoOutputOutputStreamException :exception {};
	static unordered_map <string, Ty_TokenKind> tokenKindStr2Num;
	static vector<string> tokenKindNum2Str;
	void InitLex();
	Lexeme();
	void SetInput(ios& in) { this->input = &in; }
	vector<Token> Analyse() const;
};