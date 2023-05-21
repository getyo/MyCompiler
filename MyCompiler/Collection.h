#pragma once
#include "Production.h"
#include <list>
#include <vector>
#include "Grammer.h"
#include <memory>
#include <map>
#include <unordered_set>
#include <unordered_map>

typedef list<Item> ItemSet;
typedef map<int, Item*> InputMap;

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
		Pair(Item* itemPtr, int status) :itemPtr(itemPtr), status(status) {}
	};
	struct ItemIndex {
		int status;
		int index;
		ItemIndex(){}
		ItemIndex(int status, int index) :status(status), index(index) {}
	};
	struct ItemIndexEqual {
		bool operator()(const ItemIndex& i1, const ItemIndex& i2)const {
			if (i1.status != i2.status) return false;
			if (i1.index != i2.index) return false;
			return true;
		}
	};
	struct ItemIndexHash {
		int operator()(const ItemIndex &i)const {
			return hash<int>()(i.status) && hash<int>()(i.index);
		}
	};

	//保存从某个Item能到其他那些Item
	unordered_map<ItemIndex, vector<Pair>, ItemIndexHash, ItemIndexEqual> fromTo;

	//bool Reduciable(Item&);
	//void GrammerSymbolIsNonAble();
	bool VectorFind(vector<Pair>& vec, Item& item);
	int HasItemSet(ItemSet& itemSet);
	bool HasItem(ItemSet& itemSet, Item& item);
	bool HasItem(vector<Item>& itemSet, Item& item);

	set<int> FirstTerminalAfterDot(Item);
	vector<Item> ClosureLR1(Item& item);
	void IntiLookAhead();
	void RemoveAtHead();
	void LookAheadPorpagate();


	void ClosureLR0(ItemSet&);
	int InputSymbol(int statusNum, int symbolID);
	void RemoveNonKernel();
	void InsertItemSet();
	void ConstructLR0();


	Collection();
	Collection(const Collection&);
	~Collection();
	void operator=(const Collection&) {}
public:
	static const int NON_ENTRY;
	static const int LOOKAHEAD_ATHAND;
	static const int ACCESS;
	static Collection* CollectionFactory();
	Grammer* GetGrammer();
	int Goto(int curStatus, int symbol);
	string Info();
	ItemSet& GetStatus(int status) {return collection[status];}
	bool HasFollowDot(int preStatus,int inputSymbol){
		for (auto& i : collection[preStatus]) {
			if (i.FollowDot() == inputSymbol) return true;
		}
		return false;
	}
	bool HasDotPos1(int curStatus) {
		for (auto& i : collection[curStatus]) {
			if (i.GetDotPos() == 1) return true;
		}
		return false;
	}
	void Print();
	void PrintStatus(int status);

	static void Release() {
		Grammer::Release();
		if (collectionPtr != nullptr) {
			delete collectionPtr;
			collectionPtr = nullptr;
		}
	}
};