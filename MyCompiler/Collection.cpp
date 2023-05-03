#include "Collection.h"
#include <iostream>
#include <stack>
#include <queue>
#include "Debug.h"
using namespace std;

Collection* Collection::collectionPtr = nullptr;
const int Collection::LOOKAHEAD_ATHAND = -3;
const int Collection::NON_ENTRY = 99999999;
//用第0号表达式进行归约即成功
const int Collection::ACCESS = -1;

Collection* Collection::CollectionFactory() {
	if (collectionPtr == nullptr) collectionPtr = new Collection();
	return collectionPtr;
}

void Collection::InsertItemSet() {
	collection.push_back(ItemSet());
	parserTable.push_back(new int[grammerSymbolCnt]);
	auto line = parserTable[parserTable.size() - 1];
	for (int i = 0; i < grammerSymbolCnt; i++) {
		line[i] = NON_ENTRY;
	}
}

//如果有返回下标，没有返回-1
int Collection::HasItemSet(ItemSet& itemSet) {
	for (int i = 0; i < collection.size(); i++) {
		if (collection[i] == itemSet) return i;
	}
	return -1;
}

//如果能到达状态，返回状态号，否则返回-1
int Collection::InputSymbol(ItemSet& itemSet, int symbolID) {
	ItemSet tempStatus;
	for (auto& i : itemSet) {
		if (i.FollowDot() == symbolID) {
			tempStatus.push_back(Item(i, i.GetDotPos() + 1));
		}
	}
	if (tempStatus.size() == 0) return -1;
	ClosureLR0(tempStatus);
	int gotoStatus = HasItemSet(tempStatus);
	if (gotoStatus != -1) return gotoStatus;
	InsertItemSet();
	*(--collection.end()) = tempStatus;
	return collection.size() - 1;
}

bool Collection::HasItem(ItemSet& itemSet, Item& item) {
	for (auto itemPtr = itemSet.begin(); itemPtr != itemSet.end(); itemPtr++) {
		if (*itemPtr == item) return true;
	}
	return false;
}

void Collection::ClosureLR0(ItemSet& itemSet) {
	for (auto item = itemSet.begin(); item != itemSet.end(); item++) {
		for (int j = 0; j < grammer->ProductionCnt(); j++) {
			Production& p = (*grammer)[j];
			if (p.GetHead() == item->FollowDot()) {
				Item temp(p, j, 0, {});
				if (!HasItem(itemSet, temp)) itemSet.push_back(temp);
			}
		}
	}
}

void Collection::ConstructLR0() {
	for (int i = 0; i < collection.size(); i++) {
		ClosureLR0(collection[i]);

		for (int j = 0; j < grammer->GrammerSymbolCnt(); j++) {
			int gotoStatus = InputSymbol(collection[i], j);
			if (gotoStatus != -1) parserTable[i][j] = gotoStatus;
		}
	}
}

void Collection::RemoveNonKernel() {
	vector<ItemSet::const_iterator> unkernel;
	for (auto& itemSet : collection) {
		for (auto itemPtr = itemSet.begin(); itemPtr != itemSet.end(); ++itemPtr)
			if (itemPtr->GetHead() != grammer->StartSymbol() && \
				!itemPtr->GetDotPos())
				unkernel.push_back(itemPtr);
		for (auto& itemPtr : unkernel) itemSet.erase(itemPtr);
		unkernel.clear();
	}
}

set<int> Collection::FirstTerminalAfterDot(Item item) {
	set<int> s;
	item.AddDotPos();
	int symbol = item.FollowDot();

	if (symbol == Item::BLANK_FOLLOW_DOT) {
		for (auto& t : item.GetLookAheadSet())
			s.insert(t);
	}
	else if (grammer->IsTerminal(symbol))
		s.insert(symbol);
	else if (grammer->IsUnterminal(symbol)) {
		stack<int> unterminalStack;
		unterminalStack.push(symbol);
		while (!unterminalStack.empty()) {
			symbol = unterminalStack.top();
			unterminalStack.pop();
			for (int i = 0; i < grammer->ProductionCnt(); i++) {
				auto p = grammer->operator[](i);
				auto body = p.GetBody();
				if (p.GetHead() == symbol) {
					if (grammer->IsTerminal(body[0]))
						s.insert(body[0]);
					else if (grammer->IsUnterminal(body[0]))
						unterminalStack.push(body[0]);
				}
			}
		}
	}
	return s;
}

vector<Item> Collection::ClosureLR1(Item& item) {
	vector<Item> itemVec;
	Item temp = item;
	temp.AddLookAhead(LOOKAHEAD_ATHAND);
	itemVec.push_back(temp);

	for (int i = 0; i < itemVec.size(); i++) {
		Item item = itemVec[i];
		int followDot = item.FollowDot();
		if (followDot == Item::BLANK_FOLLOW_DOT) continue;
		if (grammer->IsTerminal(followDot)) continue;
		for (int i = 0; i < grammer->ProductionCnt(); i++) {
			Production p = grammer->operator[](i);
			if (followDot == p.GetHead()) {
				Item temp(p, i);
				auto s = FirstTerminalAfterDot(item);
				for (auto& i : s) temp.AddLookAhead(i);
				itemVec.push_back(temp);
			}
		}
	}
	return itemVec;
}

int Collection::Goto(int curStatus, int symbol) {
	return parserTable[curStatus][symbol];
}

bool Collection::VectorFind(vector<Pair>& vec, Item& item) {
	for (auto i : vec)
		if (Item::ProductionEqual(item,*i.itemPtr) && \
			item.GetDotPos() == i.itemPtr->GetDotPos()) return true;
	return false;
}
void Collection::AddFromTo(Item& from, int status) {
	queue<int> headQueue;
	unordered_set<Item, ItemHash, ItemEqual> closure;
	int followDot = from.FollowDot();
	if (grammer->IsUnterminal(followDot)) headQueue.push(followDot);
	if (followDot >= 0)closure.insert(Item(from, from.GetDotPos()));
	else return;
	//对from求LR0闭包
	int head,bodyStart;
	while (!headQueue.empty()) {
		head = headQueue.front();
		headQueue.pop();
		for (int i = 0; i < grammer->ProductionCnt(); i++) {
			auto p = (*grammer)[i];
			if (p.GetHead() == head) {
				closure.insert(Item(p, i));
				if (grammer->IsUnterminal(p[1]))
					headQueue.push(p[1]);
			}
		}
	}
	int s = 0;
	//根据闭包寻找kernelItem
	for (auto& i : closure) {
		s = 0;
		for (auto& itemSet : collection) {
			for (auto& kernelItem : itemSet) {
				if (Item::ProductionEqual(kernelItem, i) && kernelItem.IsDerived(i)) {
					if (!VectorFind(fromTo[from], kernelItem))
						fromTo[from].push_back(Pair(&kernelItem, s));
				}
			}
			++s;
		}
	}
}

void Collection::IntiLookAhead() {
	int status = 0;
	vector<ItemSet> tempCollection = collection;
	
	for (auto& itemSet : tempCollection) {
		for (auto& kernelItem : itemSet) {
			AddFromTo(kernelItem, status);
			if (kernelItem.FollowDot() == Item::BLANK_FOLLOW_DOT) continue;
			kernelItem.AddLookAhead(LOOKAHEAD_ATHAND);
			auto closure = ClosureLR1(kernelItem);
			for (auto& item : closure) {
				//对于当前Item的每一个lookAhead
				for (auto lookAhead : item.GetLookAheadSet()) {
					int followDot = item.FollowDot();
					int nextStatus = Goto(status, followDot);
					//如果对于当前followDot已有归约操作，
					if (nextStatus < 0) {
						cerr << "Shift Reduction Conflication\n";
						cerr << "Item:\n";
						item.Print();
						cerr << "\nShift Symbol : " << grammer->grammerSymbolNum2Str[followDot] << "\n";

						cerr << "\nOrigin Reduce Production:\n";
						(*grammer)[-(nextStatus + 1)].Print();
						cerr << "\n";

						cerr << "\nCurrent Status: Status " << status << "\n";
						PrintStatus(status);
						cerr << "\n";
						exit(1);
					}

					ItemSet& gotoStatus = collection[nextStatus];
					for (auto& i : gotoStatus) {
						//向对应kernel Item添加lookAhead
						if (i.IsDerived(item)) {
							i.AddLookAhead(lookAhead);
							//如果当前dot已经到了最后位置,添加reduce操作
							if (i.FollowDot() == Item::BLANK_FOLLOW_DOT && lookAhead != LOOKAHEAD_ATHAND) {
								if (parserTable[nextStatus][lookAhead] == NON_ENTRY)
									parserTable[nextStatus][lookAhead] = (-i.GetPItr());
								else if (parserTable[nextStatus][lookAhead] >= 0) {
									cerr << "\nGammer is not LALR!\n";
									cerr << "Shift Reduction Conflication\n";
									cerr << "InputSymbol : " << grammer->grammerSymbolNum2Str[lookAhead] << "\n";

									cerr << "\nCurrent Status: Status " << nextStatus << "\n";
									PrintStatus(nextStatus);
									cerr << "\n";

									cerr << "\nOrigin Goto Status: Status " << parserTable[nextStatus][lookAhead] << '\n';
									PrintStatus(parserTable[nextStatus][lookAhead]);
									cerr << "\n";

									cerr << "\nCurrect Reduce Production: \t";
									(*grammer)[i.GetPItr() - 1].Print();
									cerr << "\n";

									exit(1);

								}
								else {
									if (parserTable[nextStatus][lookAhead] == (-i.GetPItr())) continue;
									cerr << "\nGammer is not LALR!\n";
									cerr << "Reduction Reduction Conflication\n";
									cerr << "InputSymbol : " << grammer->grammerSymbolNum2Str[lookAhead] << "\n";

									cerr << "\nCurrent Status: Status " << nextStatus << "\n";
									PrintStatus(nextStatus);
									cerr << "\n";

									cerr << "\nOrigin Reduce Production: \t";
									(*grammer)[-(parserTable[nextStatus][lookAhead]+1)].Print();
									cerr << "\n";

									cerr << "\nCurrect Reduce Production: \t";
									(*grammer)[i.GetPItr() - 1].Print();
									cerr << "\n";

									exit(1);
								}
							}
							if (lookAhead == LOOKAHEAD_ATHAND)
								porpagateTable[i].insert(status);
						}
					}
				}

			}
		}
		++status;
	}
	//向起始符号所在的item添加$
	int end = grammer->grammerSymbolStr2Num["$"];
	collection[0].begin()->AddLookAhead(end);
	parserTable[0][end] = ACCESS;
	//Print();
	//cout << "\n\n\n";
}

void Collection::RemoveAtHead() {
	for (auto& itemSet : collection) {
		for (auto& i : itemSet)
			if (i.FindLookAhead(LOOKAHEAD_ATHAND))
				i.RemoveLookAhead(LOOKAHEAD_ATHAND);
	}
}

void Collection::LookAheadPorpagate() {
	int status = 0;
	//遍历所有kernel Item
	for (auto& itemSet : collection) {
		for (auto& from : itemSet) {
			//如果当前Item本身存在lookAhead
			if (from.GetLookAheadSet().size()) {
				if (!fromTo.count(from)) continue;
				//将当前Item的lookAhead传播到含有LOOKAHEAD_ATHEAD的item
				for (auto to : fromTo[from]) {
					if (!to.itemPtr->FindLookAhead(LOOKAHEAD_ATHAND) || \
						!porpagateTable[*to.itemPtr].count(status))
						continue;
					to.itemPtr->RemoveLookAhead(LOOKAHEAD_ATHAND);
					for (int lookAhead : from.GetLookAheadSet()) {
						if (lookAhead == LOOKAHEAD_ATHAND) continue;
						to.itemPtr->AddLookAhead(lookAhead);
						//如果当前dot已经到了最后位置,添加reduce操作
						if (to.itemPtr->FollowDot() == Item::BLANK_FOLLOW_DOT)
							if (parserTable[to.status][lookAhead] == NON_ENTRY)
								parserTable[to.status][lookAhead] = (-to.itemPtr->GetPItr());
							else if (parserTable[to.status][lookAhead] == (-to.itemPtr->GetPItr())) continue;
							else {
								cerr << "\nGammer is not LALR!\n";
								if (parserTable[to.status][lookAhead] > 0) {
									cerr << "Shift Reduction Conflication\n";
									cerr << "Item:\n";
									to.itemPtr->Print();
									cerr << "\nShift Symbol : " << grammer->grammerSymbolNum2Str[lookAhead] << "\n";

									cerr << "\n";
									cerr << "\nFrom Status: Status " << to.status << "\n";
									PrintStatus(status);
									cerr << "\n";


									cerr << "\nCurrent Status: Status " << to.status << "\n";
									PrintStatus(to.status);
									cerr << "\n";

									cerr << "\nOrigin Goto Status: Status " << parserTable[to.status][lookAhead] << '\n';
									PrintStatus(parserTable[to.status][lookAhead]);
									cerr << "\n";

									cerr << "\nCurrect Reduce Production: \t";
									(*grammer)[to.itemPtr->GetPItr() - 1].Print();
									cerr << "\n";
								}
								else {
									cerr << "Reduction Reduction Conflication\n";
									cerr << "Item:\n";
									to.itemPtr->Print();
									cerr << "\nInput Symbol : " << grammer->grammerSymbolNum2Str[lookAhead] << "\n";

									cerr << "\n";
									cerr << "\nFrom Status: Status " << to.status << "\n";
									PrintStatus(status);
									cerr << "\n";

									cerr << "\n";
									cerr << "\nCurrent Status: Status " << to.status << "\n";
									PrintStatus(to.status);

									cerr << "\n";
									cerr << "\nOrigin Reduce Production: \t";
									(*grammer)[-(parserTable[to.status][lookAhead] + 1)].Print();
									cerr << '\n';

									cerr << "\nCurrect Reduce Production: \t";
									(*grammer)[to.itemPtr->GetPItr() - 1].Print();
									cerr << "\n";
								}
								cerr << '\n';
								exit(1);
							}
					}
				}
			}
		}
		++status;
	}
	RemoveAtHead();
}


Collection::Collection() {
	//创建第一个状态，并向其中加入第一个Item
	grammer = Grammer::GrammerFactory();
#ifdef _GRAMMER_PRINT
	grammer->Print();
#endif // _GRAMMER_PRINT

	grammerSymbolCnt = grammer->GrammerSymbolCnt();

	InsertItemSet();
	collection[0].push_back(Item((*grammer)[0], 0, {}));

	//构建LR（0）Item集合；
	ConstructLR0();
	//Print();
	//把非Kernel状态去除
	RemoveNonKernel();
#ifdef _LR0_PRINT
	Print();
	cout << "\n\n";
#endif // _LR0_PRINT

	//初始化kernel Item的lookahead
	IntiLookAhead();
	//构建LALR分析表
	LookAheadPorpagate();
}

void Collection::PrintStatus(int status) {
#ifdef DEBUG
	ASSERT(status < collection.size(), "Incorrect Status");
#endif // DEBUG
	for (auto& i : collection[status]) {
		i.Print();
		cout << '\n';
	}
	auto line = parserTable[status];
	cout << "Goto: \t";
	for (int j = 0; j < grammerSymbolCnt; j++) {
		if (line[j] != NON_ENTRY && line[j] >= 0)
			cout << Grammer::grammerSymbolNum2Str[j] \
			<< " " << line[j] << " \t";
	}
	cout << "\n";
	cout << "Reduce: \t";
	for (int j = 0; j < grammerSymbolCnt; j++) {
		if (line[j] != NON_ENTRY && line[j] < 0)
			cout << Grammer::grammerSymbolNum2Str[j] \
			<< " " << line[j] << " \t";
	}	
}

void Collection::Print() {
	int i = 0;
	for (auto& itemSet : collection) {
		cout << "Status" << i << ":\n";
		for (auto& i : itemSet) {
			i.Print();
			cout << '\n';
		}
		auto line = parserTable[i];
		cout << "Goto: \t";
		for (int j = 0; j < grammerSymbolCnt; j++) {
			if (line[j] != NON_ENTRY && line[j] >= 0)
				cout << Grammer::grammerSymbolNum2Str[j] \
				<< " " << parserTable[i][j] << " \t";
		}
		cout << "\n";
		cout << "Reduce: \t";
		for (int j = 0; j < grammerSymbolCnt; j++) {
			if (line[j] != NON_ENTRY && line[j] < 0)
				cout << Grammer::grammerSymbolNum2Str[j] \
				<< " " << parserTable[i][j] << " \t";
		}
		cout << "\n\n";
		++i;
	}
}

Grammer* Collection::GetGrammer() {
	return grammer;
}

Collection::~Collection() {
	if (collectionPtr != nullptr) {
		for (auto& p : parserTable) delete[] p;
	}
}