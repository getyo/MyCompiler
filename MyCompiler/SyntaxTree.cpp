#include "SyntaxTree.h"
#include <queue>
#include "Debug.h"
static int regStrIndex;		//正则表达式字符串当前位置索引
static int curPos;				//叶子节点pos计数
static int curTreeID;			//当前正在构造的语法树ID
static string curReg;			//现在需要分析的正则表达式字符串
vector<vector<int>> SyntalTree::leftNodeTable;
static int treeID = 0;

static int GetTreeID() {
	return treeID++;
}
bool SyntalNode::Nullable(SyntalNodePtr node) {
	if (!node->leftChild && !node->rightChild && node->name != '\0') {
		node->nullable = false;
		return false;
	}
	else if (node->name == STAR_NODE ||
		node->leftChild->name == '\0' ||
		node->rightChild->name == '\0' ||
		node->name == '\0') {
		node->nullable = true;
		return true;
	}
	else {
		node->nullable = false;
		return false;
	}
}

void SyntalNode::FirstPos(SyntalNodePtr node) {
	if (!node->leftChild && !node->rightChild)
		node->firstPos.insert(node->pos);
	else {
		switch (node->name)
		{
		case STAR_NODE: {
			//StarNode的firstPos就是其子节点的firstPos
			node->firstPos.insert(node->leftChild->firstPos.begin(), \
				node->leftChild->firstPos.end());
			break;
		}
		case CAT_NODE: {
			node->firstPos.insert(node->leftChild->firstPos.begin(), \
				node->leftChild->firstPos.end());
			//如果CatNode左孩子可为null，说明右孩子的firstPos也可能是其firstPos
			if (node->leftChild->nullable)
				node->firstPos.insert(node->rightChild->firstPos.begin(), \
					node->rightChild->firstPos.end());
			break;
		}
		case OR_NODE: {
			node->firstPos.insert(node->leftChild->firstPos.begin(), \
				node->leftChild->firstPos.end());
			node->firstPos.insert(node->rightChild->firstPos.begin(), \
				node->rightChild->firstPos.end());
			break;
		}
		default:
			break;
		}
	}
}

void SyntalNode::LastPos(SyntalNodePtr node) {
	if (!node->leftChild && !node->rightChild)
		node->lastPos.insert(node->pos);
	else {
		switch (node->name)
		{
		case STAR_NODE: {
			node->lastPos.insert(node->leftChild->lastPos.begin(), \
				node->leftChild->lastPos.end());
			break;
		}
		case CAT_NODE: {
			node->lastPos.insert(node->rightChild->lastPos.begin(), \
				node->rightChild->lastPos.end());
			if (node->rightChild->nullable)
				node->lastPos.insert(node->leftChild->lastPos.begin(), \
					node->leftChild->lastPos.end());
			break;
		}
		case OR_NODE: {
			node->lastPos.insert(node->leftChild->lastPos.begin(), \
				node->leftChild->lastPos.end());
			node->lastPos.insert(node->rightChild->lastPos.begin(), \
				node->rightChild->lastPos.end());
			break;
		}
		default:
			break;
		}
	}
}

SyntalNodePtr SyntalNode::PrimarySymbol(SyntalNodePtr node) {
	//确定节点name
	if (curReg[regStrIndex] == '(') {
		++regStrIndex;
		node = RegularExpression(node);
#ifdef DEBUG
		if (curReg[regStrIndex] == '\0') {
			cout << "unmatched parenthese";
		}
#endif // DEBUG
		//跳过右括号
		++regStrIndex;
		return node;
	}
	else if (curReg[regStrIndex] == '\\') {
		++regStrIndex;
		if (curReg[regStrIndex] == '\\' && curReg[regStrIndex + 1] == '0') {
			node->SetName('\0');
			regStrIndex += 2;
		}
		else if (curReg[regStrIndex] == '*' ||
			curReg[regStrIndex] == '|' ||
			curReg[regStrIndex] == '(' ||
			curReg[regStrIndex] == ')' ||
			curReg[regStrIndex] == '\'' ||
			curReg[regStrIndex] == REG_END)
			node->SetName(curReg[regStrIndex]);
		else {
			cerr << "Symbol " << curReg[regStrIndex] <<
				" is not a special symbol which need to follow by \\" << endl;
			abort();
		}
	}
	else if (curReg[regStrIndex] == '\0') return nullptr;
	else node->SetName(curReg[regStrIndex++]);

	//设置叶子节点pos，并计算相应集合
	node->SetPos(curPos++);
	SyntalNode::Nullable(node);
	SyntalNode::FirstPos(node);
	SyntalNode::LastPos(node);
	SyntalTree::leftNodeTable[curTreeID].push_back(node->name);

#ifdef DEBUG
	if (SyntalTree::leftNodeTable[curTreeID].size() != (curPos)) {
		cerr << "SyntalTree " << curTreeID << " has leftchild which has unmatched pos and name : \n"
			<< "pos : " << node->pos << " name : " << node->name << "\n";
	}
#endif // DEBUG

	return node;
}

SyntalNodePtr SyntalNode::Closure(SyntalNodePtr node) {
	node = PrimarySymbol(node);
	SyntalNodePtr father = node;
	if (curReg[regStrIndex] == '*') {
		father = make_shared<SyntalNode>();
		father->SetName(STAR_NODE);
		father->SetLeftChild(node);
		SyntalNode::Nullable(father);
		SyntalNode::FirstPos(father);
		SyntalNode::LastPos(father);
		while (curReg[++regStrIndex] == '*');
	}
	return father;
}

SyntalNodePtr SyntalNode::Concatenation(SyntalNodePtr node) {
	SyntalNodePtr left = Closure(node);
	SyntalNodePtr father = left;
	SyntalNodePtr right = make_shared<SyntalNode>();
	while (curReg[regStrIndex] != '*' && curReg[regStrIndex] != '|' && curReg[regStrIndex] != '\0') {
		father = make_shared<SyntalNode>(SyntalNode());
		right = right->Closure(make_shared<SyntalNode>(SyntalNode()));
		father->SetName(CAT_NODE);
		father->SetLeftChild(left);
		father->SetRightChild(right);
		SyntalNode::Nullable(father);
		SyntalNode::FirstPos(father);
		SyntalNode::LastPos(father);
		if (curReg[regStrIndex] != '*' && curReg[regStrIndex] != '|' && curReg[regStrIndex] != '\0')
			left = father;
	}
	return father;
}

SyntalNodePtr SyntalNode::Union(SyntalNodePtr node) {
	SyntalNodePtr left = Concatenation(node);
	SyntalNodePtr father = left;
	SyntalNodePtr right = make_shared<SyntalNode>(SyntalNode());
	while (curReg[regStrIndex] == '|') {
		++regStrIndex;
		father = make_shared<SyntalNode>();
		right = right->Closure(make_shared<SyntalNode>(SyntalNode()));
		father->SetName(OR_NODE);
		father->SetLeftChild(left);
		father->SetRightChild(right);
		SyntalNode::Nullable(father);
		SyntalNode::FirstPos(father);
		SyntalNode::LastPos(father);
		if (curReg[regStrIndex] == '|')
			left = father;
	}
	return father;
}

SyntalNodePtr SyntalNode::RegularExpression(SyntalNodePtr node) {
	return Union(node);
}

void SyntalNode::PrintNode(SyntalNodePtr node) {
	cout << "name:";
	switch (node->name) {
	case CAT_NODE: {
		cout << "+"; break;
	}
	case OR_NODE: {
		cout << "|"; break;
	}
	case STAR_NODE: {
		cout << "*"; break;
	}
	default: {
		cout << (char)node->name; break;
	}
	}
	cout << " ";
	if (node->pos != -1) cout << "pos:" << node->pos << " ";
}

void SyntalTree::PrintTree(SyntalTreePtr tree) {
	cout << "treeId : " << tree->id << "\n";
	queue <SyntalNodePtr> q;
	q.push(tree->root);
	SyntalNodePtr temp;
	int curLine = 1, nextLine = 0;
	while (!q.empty()) {
		temp = q.front();
		SyntalNode::PrintNode(temp);
		if (temp->leftChild) {
			q.push(temp->leftChild);
			++nextLine;
		}
		if (temp->rightChild) {
			q.push(temp->rightChild);
			++nextLine;
		}
		q.pop();
		if (!(--curLine)) {
			curLine = nextLine;
			nextLine = 0;
			cout << "\n";
		}
	}
}

void SyntalTree::InitContrustTree (SyntalTreePtr tree) {
	curTreeID = tree->id;
	leftNodeTable.push_back(vector<int>());
	regStrIndex = 0;
	curPos = 0;
	curReg = *tree->reg;
}

SyntalTree::SyntalTree(string& reg) {
	this->reg = &reg;
	this->id = GetTreeID();
}

SyntalTreePtr SyntalTree::ConstructSyntalTree(string& reg) {
	SyntalTreePtr tree = make_shared<SyntalTree>(reg);
	SyntalNodePtr root = make_shared<SyntalNode>();
	InitContrustTree(tree);
	root = root->RegularExpression(root);
	tree->root = root;
	tree->maxPos = curPos - 1;
	return tree;
}

/*
int main() {
	string s = "(a|b)*abb#";
	SyntalTreePtr tree = SyntalTree::ConstructSyntalTree(s);
	SyntalTree::PrintTree(tree);
	return 0;
}
*/
