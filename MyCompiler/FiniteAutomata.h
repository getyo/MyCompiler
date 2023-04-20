#pragma once
#include <set>
#include "SyntaxTree.h"
#include "Lex.h"
#include "Debug.h"
#include <array>
using namespace std;

class Lexeme;
typedef vector<set<int>> Ty_FollowPos;
typedef  set<int> Ty_Status;
class FiniteAutomata
{
protected:
	int statusCnt;
	set <int> acceptStatus;
	virtual bool AddEdge(int from, int to, char symbol) = 0;
public:
	virtual bool IsAccept(int status) = 0;
	virtual bool HasEdgeTo(int from, int to, char symbol) = 0;
	virtual	bool HasEdge(int from, char symbol) = 0;
	virtual void Print() = 0;
	virtual ~FiniteAutomata() {}
};

class DFA : public FiniteAutomata {
private:
	vector<shared_ptr<array<int,128>>>  transitionTable;
	virtual bool AddEdge(int from, int to, char symbol);
	//�ҵ������±꣬��״̬�ţ�û�ҵ�����-1
	int FindStatus(vector<Ty_Status>&,Ty_Status&);
	void FindAccept(vector<Ty_Status>&,vector<int>&);
	void InsertStatus();
public:
	DFA() {}
	DFA(SyntalTreePtr tree,Ty_FollowPos& followPos);
	inline int EdgeTo(int from, char symbol);
	inline bool HasEdgeTo(int from, int to, char symbol);
	inline bool HasEdge(int from, char symbol);
	inline bool IsAccept(int status);
	void Print();
	virtual ~DFA() {}
};



