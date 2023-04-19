#pragma once
#include "SyntaxTree.h"
#include "Debug.h"
#include <set>
using namespace std;

typedef vector<set<int>> Ty_FollowPos;
class Lexeme {
private:
	ios* regIn;
	ios* out;
	ios* input;
	vector<Ty_FollowPos> followPosTable;
	vector <string> regArray;
	vector <SyntalTreePtr> treeArray;


	//将输入的正则表达式整理好，并且构建语法树，DAF
	void InitLex();
	void FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos);
	Lexeme() {}
public:
	static int OUTPUT_TO_SCREEN;
	static int OUTPUT_TO_FILE;
	Lexeme(ios& regIn) {
		this->regIn = &regIn;
		InitLex();
	}
	void SetInput(ios& in) { this->input = &in; }
	void SetOutput(ios& out) { this->out = &out; }
};
int Lexeme::OUTPUT_TO_SCREEN = 1;
int Lexeme::OUTPUT_TO_FILE = 2;