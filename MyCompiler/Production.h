#pragma once
#include <vector>
#include <string>
#include <set>
#include <memory>
using namespace std;
#define SYMBOL_ATTR_CNT 8
class Action;
class Production {
protected:
	int head;
	vector<int> body;
public:

	vector<shared_ptr<Action>> actions;
	Production() {}
	Production(initializer_list<int> l);
	Production(int head,vector<int>& body);
	Production(vector<int>& p);
	Production(const Production&&);
	Production(const Production&);
	int GetHead() const;
	vector<int> GetBody() const;
	int& operator[](int index);
	virtual string Info() const;
	string AttrStr(int symbolID, int attrIndex) const;
	virtual void Print() const;
	Production& operator=(const Production&);
	bool operator()(const Production&) const;
	bool operator==(const Production&) const;
	virtual ~Production() {}
};

struct Action {
	vector<int> funPtrlist;
	vector <int> requested;
	//遇到PUSH_ALL说明要把之后跟的数字（设为n），产生式中的第n个语法符号ID以及其属性指针当作参数
	static const int PUSH_ALL,ADD,MINUS,MULT,DIV,ASSIGN,FUN,REM;
	void Print();
};


class Item :public Production{
	friend bool ItemLess(const Item& lhs, const Item& rhs);
	friend struct ItemEqual;
	friend struct ItemHash;
private:
	//production在grammer中的下标
	int pItr;
	int dotPos;
	set<int> lookAhead;
public:
	static bool ProductionEqual(const Item&, const Item&);
	static int BLANK_FOLLOW_DOT;
	Item() {}
	Item(const Production& p,int pItr);
	Item(const Item&);
	explicit Item(const Item&, int dotPos);
	Item(Production& p,int pItr,initializer_list<int> l);
	Item(Production& p,int pItr,int dotPos,initializer_list<int> l);
	bool FindLookAhead(int c) const;
	void RemoveLookAhead(int c);
	void AddLookAhead(int c);
	int AddDotPos();
	set<int> GetLookAheadSet() const;
	int GetDotPos() const;
	int MaxDotPos() const;
	int FollowDot() const;
	Item& operator =(const Item&) ;
	bool operator<(const Item&) const;
	bool operator()(const Item&) const;
	bool operator==(const Item&) const;
	bool operator!=(const Item&) const;
	//判断传入的Item是不是由this派生得到，即由this的dot后移一个单位得到
	bool IsDerived(const Item&) const;
	int GetPItr() const;
	string Info() const;
	void Print() const;
	~Item() {}
};

bool ItemLess(const Item& lhs, const Item& rhs);

struct ItemEqual
{
	bool operator()(const Item& lhs, const Item& rhs)const {
		if (!Item::ProductionEqual(lhs, rhs))return false;
		if (lhs.dotPos != rhs.dotPos) return false;
		return true;
	}
};

struct ItemEqualWithLookAhead
{
	bool operator()(const Item& lhs, const Item& rhs)const {
		return lhs == rhs;
	}
};

struct ItemHash
{
	int operator()(const Item& i)const {
		int res = hash<int>()(i.head);
		for (auto& d : i.body) {
			res ^= hash<int>()(d);
		}
		res ^= i.dotPos;
		return res;
	}
};