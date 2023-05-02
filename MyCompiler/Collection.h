#pragma once
#include "Production.h"
#include <list>
#include <vector>
#include "Grammer.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>
typedef list<Item> ItemSet;

class Collection {
private:
	Grammer* grammer;
	static Collection* collectionPtr;
	size_t grammerSymbolCnt;
	vector<ItemSet> collection;
	vector<int*> parserTable;
	//int nonInt;
	//bool* isNonAble;


	struct Pair {
		Item* itemPtr;
		int status;
		Pair(Item * itemPtr,int status):itemPtr(itemPtr),status(status){}
	};
	//保存从某个Item能到其他那些Item
	unordered_map<Item, vector<Pair>, ItemHash, ItemEqual> fromTo;

	//bool Reduciable(Item&);
	//void GrammerSymbolIsNonAble();
	bool VectorFind(vector<Pair>& vec, Item& item);
	int HasItemSet(ItemSet& itemSet);
	bool HasItem(ItemSet& itemSet, Item& item);

	set<int> FirstTerminalAfterDot(Item );
	vector<Item> ClosureLR1(Item& item);
	void AddFromTo(Item& from, int status);
	void IntiLookAhead();
	void LookAheadPorpagate();


	void ClosureLR0(ItemSet &);
	int InputSymbol(ItemSet& itemSet, int symbolID);
	void RemoveNonKernel();
	void InsertItemSet();
	void ConstructLR0();


	Collection();
	Collection(const Collection&);
	void operator=(const Collection&) {}
public:
	static const int NON_ENTRY;
	static const int LOOKAHEAD_ATHAND;
	static const int ACCESS;
	static Collection* CollectionFactory();
	Grammer* GetGrammer();
	int Goto(int curStatus, int symbol);
	string Info();
	void Print();
	~Collection();
};

