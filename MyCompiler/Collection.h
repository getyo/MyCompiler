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
	size_t grammerSymbolCnt;
	Grammer* grammer;
	vector<ItemSet> collection;
	vector<int*> praserTable;

	struct Pair {
		Item* itemPtr;
		int status;
		Pair(Item * itemPtr,int status):itemPtr(itemPtr),status(status){}
	};
	//�����ĳ��Item�ܵ�������ЩItem
	unordered_map<Item, vector<Pair>, ItemHash, ItemEqual> fromTo;

	int HasItemSet(ItemSet& itemSet);
	bool HasItem(ItemSet& itemSet, Item& item);

	set<int> FirstTerminalAfterDot(Item );
	vector<Item> ClosureLR1(Item& item);
	void IntiLookAhead();
	void LookAheadPorpagate();


	void ClosureLR0(ItemSet &);
	int InputSymbol(ItemSet& itemSet, int symbolID);
	void RemoveNonKernel();
	void InsertItemSet();
	void ConstructLR0();

	int Goto(int curStatus, int terminal);

	Collection(Grammer *);
	~Collection();
	Collection(const Collection&);
	void operator=(const Collection&) {}
public:
	static const int NON_ENTRY;
	static const int LOOKAHEAD_ATHAND;
	static Collection* collectionPtr;
	static Collection* CollectionFactory(Grammer * g);
	string Info();
	void Print();
};

