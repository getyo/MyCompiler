#include "Lex.h"
#include <fstream>
#include "FileManager.h"

map <string, Ty_TokenKind> Lexeme::tokenKindStr2Num;
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
			//���Ҳ���дmap��tokenKindName��Ӧ��tokenKind
			//ʵ�������tokenKind�Ͷ���������ʽ�﷨����TreeId����һ��
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
			//ʵ�������tokenKind�Ͷ���������ʽ�﷨����TreeId����һ��
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
		//ȷ��followPos�����㹻�����ϱ����ӦPos��follow��
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

void Lexeme::Dfa2Nfa() {
	NfaPtr = make_shared<NFA>(dfaVec);
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
