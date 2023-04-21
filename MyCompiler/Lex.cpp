#include "Lex.h"
#include <fstream>
#include "FileManager.h"

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
		while (in->good()) {
			string s;
			getline(*in, s);
			regArray.push_back(s);
		}
	}
	else if (istream* in = dynamic_cast<istream*>(this->regIn)) {
		while (in->good()) {
			string s;
			getline(*in, s);
			regArray.push_back(s);
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
		dfaVec.push_back(DFA(treeArray[i], followPosTable[i]));
		//dfaVec[i].Print();
	}
}

void Lexeme::DfaVec2Nfa() {
	nfaPtr = make_shared<NFA>(dfaVec);
}

void Lexeme::Nfa2Dfa() {
	unoptimizedDaf = FiniteAutomata::Nfa2Dfa(*nfaPtr);
}

void Lexeme::InitLex() {
	InputReg();
	ConstructFollowPosTable();
	Tree2Dfa();
	DfaVec2Nfa();
	Nfa2Dfa();
	for (auto& i : dfaVec) i.Print();
	cout << "\n";
	nfaPtr->Print();
	cout << "\n";
	unoptimizedDaf.Print();
	cout << "\n";
}


int main() {
	if (!FileManager::CreateMultDir("Lex\\input")) {
		cerr << "Directory creat failed \n";
		abort();
	}
	ifstream regIn("Lex\\Input\\reg.txt");
	if (!regIn.is_open()) abort();
	Lexeme lex(regIn);
}
