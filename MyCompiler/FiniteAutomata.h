#pragma once
#include <set>
#include "SyntaxTree.h"
#include "Debug.h"
#include <array>
using namespace std;

class NFA;
class DFA;
class Lexeme;
typedef vector<set<int>> Ty_FollowPos;
typedef  set<int> Ty_Status;
class FiniteAutomata
{
protected:
	int statusCnt;
	set <int> acceptStatus;
	virtual bool AddEdge(int from, int to, char symbol) = 0;
	static set<int> Closure(NFA&, set<int>&);
public:
	set<int> GetAcceptStatus() const;
	int GetStatusCnt() const;
	bool IsAccept(int status) const;
	virtual bool HasEdgeTo(int from, int to, char symbol) const = 0;
	virtual	bool HasEdge(int from, char symbol) const = 0;
	virtual void Print() const = 0;
	virtual ~FiniteAutomata() {}
	static DFA Nfa2Dfa(NFA&);
};

class DFA : public FiniteAutomata {
	friend class FiniteAutomata;
private:
	vector<shared_ptr<array<int,128>>>  transitionTable;
	virtual bool AddEdge(int from, int to, char symbol);
	//找到返回下标，即状态号，没找到返回-1
	int FindStatus(vector<Ty_Status>&,Ty_Status&);
	void FindAccept(vector<Ty_Status>&,vector<int>&);
	void InsertStatus();
public:
	DFA() {}
	DFA(SyntalTreePtr tree,Ty_FollowPos& followPos);
	DFA(DFA&& src);
	DFA(DFA& src);
	DFA& operator=(DFA &&);
	inline int EdgeTo(int from, char symbol) const;
	inline bool HasEdgeTo(int from, int to, char symbol) const;
	inline bool HasEdge(int from, char symbol) const;
	void Print() const;
	virtual ~DFA() {}
};

class NFA :public FiniteAutomata{
private:
	vector<vector<set<int>>> transitionTable;
	bool AddEdge(int from, int to, char symbol);
public:
	NFA() {}
	NFA(vector<DFA>& dfaVec);
	const set<int> * EdgeTo(int from, char symbol);
	NFA& operator=(NFA&&);
	inline bool HasEdgeTo(int from, int to, char symbol) const;
	inline bool HasEdge(int from, char symbol) const;
	void Print() const;
	~NFA() {}
};

