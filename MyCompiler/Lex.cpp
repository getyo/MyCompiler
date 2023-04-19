#include "Lex.h"
#include <fstream>
#include "FileManager.h"
void Lexeme::InitLex() {
	if (ifstream* in = dynamic_cast<ifstream *>(this->regIn)) {
		while (in->good()){
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
	for (auto& reg : regArray) {
		auto treePtr =  SyntalTree::ConstructSyntalTree(reg);
		treeArray.push_back(treePtr);
		followPosTable.push_back(Ty_FollowPos());
		auto followSet = *followPosTable.end();
		followSet.resize(treePtr->GetMaxPos());
		auto root = treePtr->GetRoot();
		FollowPos(root,followSet);
	}
}

void Lexeme::FollowPos(SyntalNodePtr& node,Ty_FollowPos& followPos) {
	if (!node->leftChild && !node->rightChild) return;
	FollowPos(node->leftChild,followPos);
	if (node->GetName() == STAR_NODE) {
		for (auto& i : node->leftChild->lastPos) {
			followPos[i].insert(node->leftChild->firstPos.begin(),
				node->leftChild->lastPos.end());
		}
		return;
	}
	FollowPos(node->rightChild, followPos);
	if (node->GetName() == CAT_NODE) {
		for (auto& i : node->leftChild->lastPos) {
			followPos[i].insert(node->rightChild->firstPos.begin(),
				node->rightChild->lastPos.end());
		}
	}
}

/*int main() {
	if (!FileManager::CreateDir("Lex")) {
		cerr << "Directory creat failed \n";
		abort();
	}
	ifstream regIn("Lex\\Input\\reg.txt");
	//Lexeme lex(regIn);
}*/