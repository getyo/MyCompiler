#pragma once
#include <vector>
#include <string>
#include <set>
using namespace std;

class Production {
protected:
	int head;
	vector<int> body;
public:
	Production() {}
	Production(initializer_list<int> l);
	Production(int head,vector<int>& body);
	Production(vector<int>& p);
	Production(const Production&&);
	Production(const Production&);
	int GetHead() const;
	vector<int> GetBody() const;
	int& operator[](size_t index);
	virtual string Info() const;
	virtual void Print() const;
	Production& operator=(const Production&);
	bool operator()(const Production&) const;
	bool operator==(const Production&) const;
	virtual ~Production() {}
};


class Item :public Production {
	friend struct ItemEqual;
	friend struct ItemHash;
private:
	int dotPos;
	set<int> lookAhead;
public:
	static bool ProductionEqual(const Item&, const Item&);
	static int BLANK_FOLLOW_DOT;
	Item() {}
	Item(const Production& p);
	Item(const Item&);
	Item(const Item&, int dotPos);
	Item(Production& p, initializer_list<int> l);
	Item(Production& p,int dotPos,initializer_list<int> l);
	bool FindLookAhead(int c) const;
	void AddLookAhead(int c);
	int AddDotPos();
	set<int> GetLookAheadSet() const;
	int GetDotPos() const;
	int MaxDotPos() const;
	int FollowDot() const;
	Item& operator =(const Item&) ;
	bool operator()(const Item&) const;
	bool operator==(const Item&) const;
	bool operator!=(const Item&) const;
	//判断传入的Item是不是由this派生得到，即由this的dot后移一个单位得到
	bool IsDerived(const Item&) const;
	string Info() const;
	void Print() const;
	~Item() {}
};

struct ItemEqual
{
	bool operator()(const Item& lhs, const Item& rhs)const {
		if (!Item::ProductionEqual(lhs, rhs))return false;
		if (lhs.dotPos != rhs.dotPos) return false;
		return true;
	}
};

struct ItemHash
{
	size_t operator()(const Item& i)const {
		size_t res = hash<int>()(i.head);
		for (auto& d : i.body) {
			res ^= hash<int>()(d);
		}
		res ^= i.dotPos;
		return res;
	}
};