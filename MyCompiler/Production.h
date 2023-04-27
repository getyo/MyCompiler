#pragma once
#include <vector>
#include <string>
#include <set>
using namespace std;

class Production {
private:
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
	virtual string Info();
	virtual void Print();
	virtual ~Production() {}
};

class Item :public Production {
private:
	int dotPos;
	set<char> lookAhead;
public:
	Item() {}
	Item(Production&& p);
	Item(Production&& p, initializer_list<char> l);
	void AddLookAhead(char c);
	bool FindLookAhead(char c) const;
	void AddDotPos();

	~Item() {}
};