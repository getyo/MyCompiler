#pragma once
#include <vector>
#include <string>
#include <set>
#include <memory>
#include <map>
using namespace std;
#define SYMBOL_ATTR_CNT 8
#define NO_SYMBOL -1
class Action;
typedef shared_ptr<Action> ActionPtr;
typedef vector<ActionPtr> ActionList;
class Production {
protected:
	int head = NO_SYMBOL;
	vector<int> body;
public:
	map<int, ActionList> actionLists;
	Production() {}
	Production(initializer_list<int> l);
	Production(int head,vector<int>& body);
	Production(vector<int>& p);
	Production(const Production&&);
	Production(const Production&);
	int GetHead() const;
	vector<int> GetBody() const;
	int& operator[](int index);
	void PushBack(int symbol);
	virtual string Info() const;
	string AttrStr(int symbolID, int attrIndex,bool inh = false) const;
	virtual void Print() const;
	Production& operator=(const Production&);
	bool operator()(const Production&) const;
	bool operator==(const Production&) const;
	virtual ~Production() {}
};

#define MAX_PARACNT 8

struct Action {
	vector <int> requested;
	//����PUSH_ALL˵��Ҫ��֮��������֣���Ϊn��������ʽ�еĵ�n���﷨����ID�Լ�������ָ�뵱������
	//����ͬʱҲ�������������ȼ�������ʮλ����ͬ�Ĵ���ͬһ���ȼ���ʮλ��Խ�����ȼ�Խ��
	static const int DIGIT = -3, OP = -2,VAL = -4,\
		FUN = -11, \
		MULT = -21, DIV = -22, REM = -23, \
		ADD = -31, MINUS = -32, \
		ASSIGN = -41;
	void Print();
	//���op1�����ȼ�����op2������true�����򷵻�false
	static bool ComparePriority(int op1, int op2) {
		return (op1 / (-10)) < (op2 / (-10)) ? true : false;
	}
};


class Item :public Production{
	friend bool ItemLess(const Item& lhs, const Item& rhs);
	friend struct ItemEqual;
	friend struct ItemHash;
private:
	//production��grammer�е��±�
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
	//�жϴ����Item�ǲ�����this�����õ�������this��dot����һ����λ�õ�
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
