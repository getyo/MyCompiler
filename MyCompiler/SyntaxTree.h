#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <set>
#include "Debug.h"
using namespace std;
#define CAT_NODE 256
#define OR_NODE 257
#define STAR_NODE 258
#define REG_END '#'

class SyntalNode;
class SyntalTree;
typedef shared_ptr<SyntalTree> SyntalTreePtr;
typedef shared_ptr<SyntalNode> SyntalNodePtr;
typedef weak_ptr <SyntalNode> WeakSyntalNodePtr;

class SyntalNode
{	
	friend class SyntalTree;
private:
	int pos;		//语法树叶子节点对应位置
	int name;		//节点符号名称，对于0-255就是原始字符，256连接，257或，258闭包
	//计算相应集合
	static bool Nullable(SyntalNodePtr);
	static void FirstPos(SyntalNodePtr);
	static void LastPos(SyntalNodePtr);

	//分析正则表达式

	SyntalNodePtr RegularExpression(SyntalNodePtr node);
	static void PrintNode(SyntalNodePtr node);
	SyntalNodePtr Union(SyntalNodePtr node);
	SyntalNodePtr Concatenation(SyntalNodePtr node);
	SyntalNodePtr Closure(SyntalNodePtr node);
	SyntalNodePtr PrimarySymbol(SyntalNodePtr node);

public:
	SyntalNodePtr leftChild;
	SyntalNodePtr rightChild;
	bool nullable;
	set<int> firstPos;
	set<int> lastPos;

	//构造语法树节点使用
	SyntalNode() :pos(-1), name(-1), leftChild(nullptr), rightChild(nullptr) {};
	inline int SetPos(int pos) {
		this->pos = pos;
		return pos;
	}

	inline int SetName(int name) {
		this->name = name;
		return name;
	}

	inline bool SetLeftChild(SyntalNodePtr l) {
		if (leftChild != nullptr) {
			return false;
		}
		else {
			this->leftChild = l;
			return true;
		}
	}

	inline bool SetRightChild(SyntalNodePtr r) {
		if (rightChild != nullptr) {
			return false;
		}
		else {
			this->rightChild = r;
			return true;
		}
	}

	inline int GetPos() const { return this->pos; }
	inline int GetName() const { return this->name; }
	~SyntalNode() {}
};

class SyntalTree {
private:

	int maxPos;
	string* reg;	//指向正则表达式的指针
	int id;			//语法树索引
	SyntalNodePtr root;

public:
	//根据语法树索引和叶节点位置找到对应符号
	static vector<vector<int>> leftNodeTable;
	static void PrintTree(SyntalTreePtr node);
	static void InitContrustTree(SyntalTreePtr tree);

	SyntalTree() {}
	SyntalTree(string& reg);
	inline SyntalNodePtr GetRoot() const{ return root; }
	inline int GetMaxPos() const{ return maxPos; }
	inline int GetID() const{ return id; }
	inline string* GetStr() const{ return reg; }


	static SyntalTreePtr ConstructSyntalTree(string& reg);
};
