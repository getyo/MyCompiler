#pragma once
#include <set>
#include "SyntaxTree.h"
#include "Debug.h"
#include <array>
#include "Token.h"
#include <unordered_map>
using namespace std;

class NFA;
class DFA;
typedef vector<set<int>> Ty_FollowPos;
typedef Ty_FollowPos Ty_StatusVec;
typedef  set<int> Ty_Status;
class FiniteAutomata
{
protected:
	//根据acceptStatus的状态号映射到该状态接受的TokenKind
	unordered_map<int, Ty_TokenKind> acceptTokenTable;
	int statusCnt;
	set <int> acceptStatus;
	virtual bool AddEdge(int from, int to, char symbol) = 0;
	static set<int> Closure(NFA&, set<int>&);
public:
	bool SetAccpetTokenKind(int acceptStatusNum, Ty_TokenKind tokenKind);
	Ty_TokenKind GetAcceptTokenKind(int acceptStatusNum) const;
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
	void FindAccept(vector<Ty_Status>&,vector<int>&,Ty_TokenKind);
	void InsertStatus();
	static const int UNMATCHED;
public:
	DFA() {}
	DFA(SyntalTreePtr tree,Ty_FollowPos& followPos,Ty_TokenKind);
	DFA(DFA&& src);
	DFA(DFA& src);
	Ty_TokenKind Recognize(string &word,int &ptr) const;
	DFA& operator=(DFA &&);
	inline int EdgeTo(int from, char symbol) const;
	inline bool HasEdgeTo(int from, int to, char symbol) const;
	inline bool HasEdge(int from, char symbol) const;
	void Print() const;
	//返回需要记录在文件中的信息
	string Info();
	virtual ~DFA() {}
	static DFA ReadDfa(ifstream& in);
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

