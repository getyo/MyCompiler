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
		//ȷ��followPos�����㹻�����ϱ����ӦPos��follow��
		followPos->insert(followPos->begin(), treePtr->GetMaxPos() + 1, set<int>());
		auto root = treePtr->GetRoot();
		SyntalTree::PrintTree(treePtr);
		FollowPos(root, *followPos);
	}
}

void Lexeme::Tree2DFA() {
	int treeCnt = treeArray.size();
	for (size_t i = 0; i < treeCnt; i++)
	{
		DfaVec.push_back(DFA(treeArray[i], followPosTable[i]));
		DfaVec[i].Print();
	}
}

void Lexeme::InitLex() {
	InputReg();
	ConstructFollowPosTable();
	Tree2DFA();
}


int main() {
	if (!FileManager::CreateDir("Lex\\input")) {
		cerr << "Directory creat failed \n";
		abort();
	}
	ifstream regIn("Lex\\Input\\reg.txt");
	Lexeme lex(regIn);
}
