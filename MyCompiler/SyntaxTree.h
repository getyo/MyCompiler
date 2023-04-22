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
	int pos;		//�﷨��Ҷ�ӽڵ��Ӧλ��
	int name;		//�ڵ�������ƣ�����0-255����ԭʼ�ַ���256���ӣ�257��258�հ�
	//������Ӧ����
	static bool Nullable(SyntalNodePtr);
	static void FirstPos(SyntalNodePtr);
	static void LastPos(SyntalNodePtr);

	//����������ʽ

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

	//�����﷨���ڵ�ʹ��
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
	string* reg;	//ָ��������ʽ��ָ��
	int id;			//�﷨������
	SyntalNodePtr root;

public:
	//�����﷨��������Ҷ�ڵ�λ���ҵ���Ӧ����
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
