#include "Production.h"
#include "Debug.h"
#include <string>
#include "Grammer.h"
#include "Collection.h"

int Item::BLANK_FOLLOW_DOT = -100000000;

Production::Production(const Production&& p) {
	this->head = p.head;
	this->body = p.body;
	this->actions = p.actions;
}

Production::Production(const Production& p) {
	head = p.head;
	body = p.body;
	this->actions = p.actions;
}

Production::Production(vector<int>& p) {
	this->head = p[0];
	this->body.insert(body.begin(), p.begin() + 1, p.end());
}

Production::Production(initializer_list<int> l) {
	this->head = *l.begin();
	for (auto it = l.begin() + 1; it != l.end(); it++)
		body.push_back(*it);
}

Production::Production(int head, vector<int>& body) {
	this->head = head;
	this->body = body;
}

int Production::GetHead() const {
	return this->head;
}

vector<int> Production::GetBody() const {
	return this->body;
}

int& Production::operator[](int index) {
	if (!index) return head;
	else return body[index - 1];
}

string Production::Info() const {
	string s = "";
	s += (to_string(head) + " ");
	for (auto& i : body) {
		s += (to_string(i) + " ");
	}
	return s;
}

string Production::AttrStr(int smbID,int attrIndex) const {
	if (Grammer::IsTerminal(smbID)) {
		switch (attrIndex)
		{
		case 0: {return "lexeme"; }
		case 1: {return "val"; }
		case 2: {return "row"; }
		case 3: {return "column"; }
		case 4: {return "typeID"; }
		default:
			break;
		}
	}
	else if (Grammer::IsUnterminal(smbID)) {
		return Grammer::GetAttrStr(smbID, attrIndex);
	}
}
void Production::Print() const {
	cout << Grammer::GetSymbolStr(head) \
		<< ' ' << "-> ";
	for (auto& i : body)
		cout << Grammer::GetSymbolStr(i)<< " ";
	if (!actions.size())
		return;
	cout << "\n{ ";
	int a;
	for (auto& action : actions) {
		for (int i = 0; i < action->requested.size(); ++i) {
			a = action->requested[i];
			switch (a)
			{
			case Action::ADD: { cout << "+ "; break; }
			case Action::ASSIGN: { cout << "= "; break; }
			case Action::DIV: { cout << "/ "; break; }
			case Action::MINUS: { cout << "- "; break; }
			case Action::MULT: { cout << "* "; break; }
			case Action::REM: { cout << "% "; break; }
			case Action::PUSH_ALL: { cout << "$" << \
				Grammer::GetSymbolStr(action->requested[++i]) << " ";
				break;
			}
			case Action::FUN: {
				int funID = action->requested[++i];
				cout << Grammer::GetFunName(funID) << "(";
				
				int symbolIndex, attrIndex;
				for (int pCnt = 0; pCnt < Grammer::GetFunParaCnt(funID); ++pCnt) {
					symbolIndex = action->requested[++i];
					attrIndex = action->requested[++i];

					if (symbolIndex == Action::PUSH_ALL) {
						cout << "$" << \
						Grammer::GetSymbolStr(body[attrIndex]);
						break;
					}

					if (symbolIndex == 0)
						cout << Grammer::GetSymbolStr(head) + '.' + \
						AttrStr(head,attrIndex);
					else 
						cout << Grammer::GetSymbolStr(body[symbolIndex - 1]) + '.'+ \
						AttrStr(body[symbolIndex-1],attrIndex);
					
					if (pCnt < (Grammer::GetFunParaCnt(funID) - 1))
						cout << ",";
				}
				cout << ")";
				break; }
			default: {
				int symbolIndex = action->requested[i++];
				int attrIndex = action->requested[i];
				if (symbolIndex == 0) cout << Grammer::GetSymbolStr(head) + '.' + \
					AttrStr(head, attrIndex);
				else cout << Grammer::GetSymbolStr(body[symbolIndex - 1]) + '.' + \
					AttrStr(body[symbolIndex - 1], attrIndex);
				cout << " ";
				break;
			}
			}
		}
		cout << "; \t";
	}
	cout << "}";
}

Production& Production::operator=(const Production& p) {
	this->head = p.head;
	this->body = p.body;
	this->actions = p.actions;
	return *this;
}

bool Production::operator()(const Production& p)const {
	return this->operator==(p);
}

bool Production::operator==(const Production& p) const {
	if (this->head != p.head) return false;
	if (this->body.size() != p.body.size()) return false;
	for (int i = 0; i < body.size(); i++)
		if (body[i] != p.body[i]) return false;
	return true;
}

int Item::FollowDot()const {
#ifdef DEBUG
	ASSERT(!(dotPos > body.size()), "dotPos out of range");
#endif // DEBUG
	if (dotPos == body.size()) return BLANK_FOLLOW_DOT;
	return this->body[dotPos];
}

int Item::MaxDotPos()const{
	return this->body.size()+1;
}

int Item::GetDotPos() const {
	return dotPos;
}

Item::Item(const Production& p,int pItr) :Production(p),pItr(pItr) {
	//默认点在产生式体开头，利用body[]可以直接返回dot后的语法符号
	this->dotPos = 0;
}

Item::Item(Production& p,int pItr,initializer_list<int> l) : Production(p),pItr(pItr) {
	this->dotPos = 0;
	for (auto c : l) lookAhead.insert(c);
}

Item::Item(Production& p,int pItr, int dotPos, initializer_list<int> l) : Item(p, pItr,l) {
	this->dotPos = dotPos;
}

Item::Item(const Item& item) {
	this->head = item.head;
	this->body = item.body;
	this->dotPos = item.dotPos;
	this->lookAhead = item.lookAhead;
	this->pItr = item.pItr;
}

Item::Item(const Item& item, int dotPos) :Item(item) {
	this->dotPos = dotPos;
}

bool Item::FindLookAhead(int c) const {
	return lookAhead.count(c);
}

void Item::RemoveLookAhead(int c) {
	lookAhead.erase(c);
}

Item& Item::operator=(const Item& item) {
	this->head = item.head;
	this->body = item.body;
	this->dotPos = item.dotPos;
	this->lookAhead = item.lookAhead;
	this->pItr = item.pItr;
	return *this;
}

bool Item::operator==(const Item& item) const {
	bool b = Production::operator==(item);
	if (!b) return false;
	if (dotPos != item.dotPos) return false;
	if (lookAhead != item.lookAhead) return false;
	return true;
}

bool Item::operator!=(const Item& item) const {
	return !(*this == item);
}

bool Item::operator<(const Item& item) const {
	if (pItr > item.pItr) return false;
	else if (pItr < item.pItr) return true;
	if (dotPos > item.dotPos) return false;
	else if (dotPos < item.dotPos) return true;
	if (lookAhead.size() > item.lookAhead.size()) return false;
	else if (lookAhead.size() < item.lookAhead.size()) return true;
	auto it1 = lookAhead.begin();
	auto it2 = item.lookAhead.begin();
	for (; it1 != lookAhead.end(); ++it1, ++it2)
		if (*it1 > *it2) return false;
		else if (*it1 < *it2) return true;
	return false;
}

bool Item::operator()(const Item& item) const {
	return this->operator==(item);
}

void Item::AddLookAhead(int c) {
	lookAhead.insert(c);
}

int Item::AddDotPos() {
	return ++dotPos;
}

set<int> Item::GetLookAheadSet() const{
	return lookAhead;
}

bool Item::ProductionEqual(const Item& i1, const Item& i2) {
	return i1.Production::operator==(i2);
}

bool Item::IsDerived(const Item& origin) const{
	if (origin.dotPos == (dotPos - 1) && ProductionEqual(*this, origin))
		return true;
	return false;
}


int Item::GetPItr() const{
	return pItr+1;
}

string Item::Info() const {
	string s = "";
	s += Production::Info();
	s += '\n';
	s += to_string(dotPos);
	for (auto& c : lookAhead) {
		s += (" " + c);
	}
	return s;
}

void Item::Print() const {
	Production::Print();
	cout << " \tdotPos: " << dotPos << " \tlookAhead: ";
	for (auto& terminal : lookAhead) {
		if (terminal != Collection::LOOKAHEAD_ATHAND)
			cout << Grammer::GetSymbolStr(terminal) << " ";
		else
			cout << '#' << " ";
	}
}

bool ItemLess(const Item& lhs, const Item& rhs) {
	if (lhs.pItr < rhs.pItr)return true;
	else if (lhs.pItr > rhs.pItr) return false;
	if (lhs.dotPos < rhs.dotPos) return true;
	else if (lhs.dotPos > rhs.dotPos) return false;
	return false;
}
