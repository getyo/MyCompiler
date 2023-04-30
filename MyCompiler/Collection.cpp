#include "Collection.h"
#include <iostream>
#include <stack>
Collection* Collection::collectionPtr = nullptr;
const int Collection::LOOKAHEAD_ATHAND = -3;
const int Collection::NON_ENTRY = 99999999;

Collection* Collection::CollectionFactory(Grammer* g) {
	if (collectionPtr == nullptr) collectionPtr = new Collection(g);
	return collectionPtr;
}

void Collection::InsertItemSet() {
	collection.push_back(ItemSet());
	praserTable.push_back(new int[grammerSymbolCnt]);
	auto line = praserTable[praserTable.size() - 1];
	for (int i = 0; i < grammerSymbolCnt; i++) {
		line[i] = NON_ENTRY;
	}
}

//����з����±꣬û�з���-1
int Collection::HasItemSet(ItemSet& itemSet) {
	for (int i = 0; i < collection.size(); i++) {
		if (collection[i] == itemSet) return i;
	}
	return -1;
}

//����ܵ���״̬������״̬�ţ����򷵻�-1
int Collection::InputSymbol(ItemSet& itemSet, int symbolID) {
	ItemSet tempStatus;
	for (auto& i : itemSet) {
		if (i.FollowDot() == symbolID)
			tempStatus.push_back(Item(i, i.GetDotPos() + 1));
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
				Item temp(p, j,0, {});
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
			if (gotoStatus != -1) praserTable[i][j] = gotoStatus;
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

	for (int i = 0; i < itemVec.size();i++) {
		Item item = itemVec[i];
		int followDot = item.FollowDot();
		if (grammer->IsTerminal(followDot)) continue;
		for (int i = 0; i < grammer->ProductionCnt(); i++) {
			Production p = grammer->operator[](i);
			if (followDot == p.GetHead()) {
				Item temp(p,i);
				auto s = FirstTerminalAfterDot(item);
				for (auto& i : s) temp.AddLookAhead(i);
				itemVec.push_back(temp);
			}
		}
	}
	return itemVec;
}

int Collection::Goto(int curStatus, int symbol) {
	return praserTable[curStatus][symbol];
}

void Collection::IntiLookAhead() {
	int status = 0;
	vector<ItemSet> tempCollection = collection;
	for (auto &itemSet : tempCollection) {
		for (auto& kernelItem : itemSet) {
			auto closure = ClosureLR1(kernelItem);
			/*
			cout << "closure:\n";
			for (auto& i : closure) {
				i.Print();
				cout << '\n';
			}
			cout << '\n' << '\n';
			*/
			//���ڱհ��е�ÿһ��Item
			for (auto& item : closure) {
				//���ڵ�ǰItem��ÿһ��lookAhead
				for (auto lookAhead : item.GetLookAheadSet()) {
					int followDot = item.FollowDot();
					if (followDot == Item::BLANK_FOLLOW_DOT) continue;
					int nextStatus = Goto(status, followDot);
					ItemSet& gotoStatus = collection[nextStatus];
					//�����ǰlookAhead���Է�����
					if (lookAhead != LOOKAHEAD_ATHAND) {
						//item.Print();
						//cout << endl;
						for (auto& i : gotoStatus) {
							//���Ӧkernel Item���lookAhead
							if (item.IsDerived(i)) {
								i.AddLookAhead(lookAhead);
								//�����ǰdot�Ѿ��������λ��,���reduce����
								if (i.FollowDot() == Item::BLANK_FOLLOW_DOT)
									praserTable[nextStatus][lookAhead] = (-i.GetPItr());
								fromTo[kernelItem].push_back(Pair(&i,nextStatus));
							}
						}
					}
					//����Ǵ�������,�����lookAhead������Ȼ���fromto
					else {
						for (auto& i : gotoStatus) {
							if (item.IsDerived(i)) {
								fromTo[kernelItem].push_back(Pair(&i,nextStatus));
							}
						}
					}
				}
			}
		}
		++status;
	}
	//����ʼ�������ڵ�item���$
	collection[0].begin()->AddLookAhead(Grammer::END_OF_GRAMMER);
	//Print();
	//cout << "\n\n\n";
}

void Collection::LookAheadPorpagate() {
	int status = 0;
	//��������kernel Item
	for (auto& itemSet : collection) {
		for (auto& from : itemSet) {
			//�����ǰItem�������lookAhead
			if (from.GetLookAheadSet().size()) {
				if (!fromTo.count(from)) continue;
				//����ǰItem��lookAhead����������
				for (auto to : fromTo[from])
					for (int lookAhead : from.GetLookAheadSet()) {
						to.itemPtr->AddLookAhead(lookAhead);
						//�����ǰdot�Ѿ��������λ��,���reduce����
						if (to.itemPtr->FollowDot() == Item::BLANK_FOLLOW_DOT) 
							praserTable[to.status][lookAhead] = (-to.itemPtr->GetPItr());
					}
			}
		}
		++status;
	}
}

Collection::Collection(Grammer* g) {
	//������һ��״̬���������м����һ��Item
	grammer = g;
	grammerSymbolCnt = grammer->GrammerSymbolCnt();
	InsertItemSet();
	collection[0].push_back(Item((*grammer)[0],0, {}));

	//����LR��0��Item���ϣ�
	ConstructLR0();
	//�ѷ�Kernel״̬ȥ��
	RemoveNonKernel();
	//��ʼ��kernel Item��lookahead
	IntiLookAhead();
	//����LALR������
	LookAheadPorpagate();
	
}

void Collection::Print() {
	int i = 0;
	for (auto& itemSet : collection) {
		cout << "Status" << i << ":\n";
		for (auto& i : itemSet) {
			i.Print();
			cout << '\n';
		}
		auto line = praserTable[i];
		cout << "Goto: \t";
		for (int j = 0; j < grammerSymbolCnt; j++) {
			if (line[j] != NON_ENTRY && line[j] >= 0)
				cout << Grammer::grammerSymbolNum2Str[j] \
				<< " " << praserTable[i][j] << " \t";
		}
		cout << "\n";
		cout << "Reduce: \t";
		for (int j = 0; j < grammerSymbolCnt; j++) {
			if (line[j] != NON_ENTRY && line[j] < 0)
				cout << Grammer::grammerSymbolNum2Str[j] \
				<< " " << praserTable[i][j] << " \t";
		}
		cout << "\n\n";
		++i;
	}
}