#include "FiniteAutomata.h"
#include <queue>
#include <stack>
#include "Lex.h"

extern SymbolTable symbolTable;
const int DFA::UNMATCHED = -1;

DFA DFA::ReadDfa(ifstream &in) {
	DFA d;
	int accCnt;
	int temp0,temp1;
	in >> d.statusCnt;
	in >> accCnt;
	for (int i = 0; i < accCnt; i++) {
		in >> temp0;
		d.acceptStatus.insert(temp0);
	}
	for (int i = 0; i < accCnt; i++) {
		in >> temp0 >> temp1;
		d.acceptTokenTable.insert({temp0,temp1});
	}
	for (int i = 0; i < d.statusCnt;i++) {
		d.transitionTable.push_back(make_shared<array<int,128>>());
		for (int j = 0; j < 128; j++) {
			in >> temp0;
			(*d.transitionTable[i])[j] = temp0;
		}
	}
	return d;
}

string DFA::Info() {
	/*
	* 格式：
	* statusCnt
	* acceptSet
	* acceptTokenTable
	* transitionTable
	*/
	string info;
	info = info + to_string(statusCnt) + "\n";
	info += (to_string(acceptStatus.size()) + " ");
	for (auto& s : acceptStatus) {
		info += (to_string(s) + " ");
	}
	info += '\n';
	for (auto& p: acceptTokenTable) {
		info += (to_string(p.first) + " ");
		info += (to_string(p.second) + " ");
	}
	info += '\n';
	for (auto line : transitionTable) {
		for (auto i : *line) {
			info += (to_string(i) + " ");
		}
		info += '\n';
	}
	return info;
}

set<int> FiniteAutomata::GetAcceptStatus() const{ 
	return acceptStatus; 
}

int FiniteAutomata::GetStatusCnt() const { 
	return this->statusCnt; 
}

bool FiniteAutomata::IsAccept(int status) const {
#ifdef DEBUG
	ASSERT(status <= this->statusCnt, "Incorrect status");
#endif // DEBUG
	return !(acceptStatus.find(status) == acceptStatus.end());
}

Ty_TokenKind FiniteAutomata::GetAcceptTokenKind(int acceptStatusNum) const{
	return acceptTokenTable.at(acceptStatusNum);
}

void DFA::Print()const {
	for (int i = 0; i < this->statusCnt; ++i) {
		cout << i << ":" << "	";
		for (int j = 0; j < 128; j++) {
			if (EdgeTo(i, j) != UNMATCHED) cout << "symbol:" << (char)j << " " \
				<< EdgeTo(i, j) << "	";
		}
		cout << "\n";
	}
	cout << "accept status :\n";
	for (auto& i : acceptStatus) cout<< "  " << i << "	" << \
		"TokenKind: " <<  Lexeme::tokenKindNum2Str[GetAcceptTokenKind(i)] << "\n";
	cout << endl;
}

bool DFA::HasEdge(int from, char symbol)const {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127, "Incorrect status");
#endif // DEBUG
	if ((*transitionTable[from])[symbol] == UNMATCHED) return false;
	return true;
}

bool DFA::HasEdgeTo(int from, int to, char symbol)const {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127 && to <= statusCnt, "Incorrect status");
#endif // DEBUG
	if ((*transitionTable[from])[symbol] == to) return true;
	return false;
}

int DFA::EdgeTo(int from, char symbol)const {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127, "Incorrect status");
#endif // DEBUG
	return (*transitionTable[from])[symbol];
}

int DFA::FindStatus(vector<Ty_Status>& statusVec, Ty_Status& s) {
	size_t size = statusVec.size();
	for (size_t i = 0; i < size; i++)
		if (statusVec[i] == s) return i;
	return -1;
}

bool DFA::AddEdge(int from, int to, char symbol) {
	if ((*transitionTable[from])[symbol] == UNMATCHED) {
		(*transitionTable[from])[symbol] = to;
		return true;
	}
	return false;
}

void DFA::InsertStatus() {
	transitionTable.push_back(make_shared<array<int, 128>>());
	auto line = *(--transitionTable.end());
	for (int i = 0; i < 128; i++) (*line)[i] = UNMATCHED;
}

void DFA::FindAccept(vector<Ty_Status>& statusVec, vector<int>& posName,Ty_TokenKind acceptedToken) {
	int size = statusVec.size();
	for (int i = 0; i < size; i++)
	{
		for (auto& pos : statusVec[i]) {
			if (posName[pos] == '#') {
				acceptStatus.insert(i);
				SetAccpetTokenKind(i, acceptedToken);
			}
		}
	}
}

DFA::DFA(SyntalTreePtr tree, Ty_FollowPos& followPos,Ty_TokenKind acceptedToken) {
	int id = tree->GetID();
	auto root = tree->GetRoot();

	//每个pos对应的char都可以用下标索引到
	auto posName = SyntalTree::leftNodeTable[id];

	queue<set<int>> statusQueue;
	vector<set<int>> statusVec;
	int curStatusNum = 0;
	int addStatusCnt = 1;
	statusQueue.push(root->firstPos);
	statusVec.push_back(root->firstPos);

	InsertStatus();
	set<int>* curStatus;
	set<int> nextStatus;

	while (!statusQueue.empty()) {
		curStatus = &statusQueue.front();
		//对于每一个输入
		int c;
		for (c = 0; c < 128; ++c) {
			for (auto& pos : *curStatus) {
				//如果当前pos和char相关联，说明输入c以后会进入到followPos(pos)
				if (posName[pos] == c)
					nextStatus.insert(followPos[pos].begin(), followPos[pos].end());
			}
			if (!nextStatus.empty()) {
				//若是已经有了该状态，添加边即可，如果没有，把该状态加入集合
				int nextStatusNum;
				if ((nextStatusNum = FindStatus(statusVec, nextStatus)) != -1) {
					AddEdge(curStatusNum, nextStatusNum, c);
				}
				else {
					InsertStatus();
					statusQueue.push(nextStatus);
					statusVec.push_back(nextStatus);
					nextStatusNum = curStatusNum + addStatusCnt;
					AddEdge(curStatusNum, nextStatusNum, c);

					if (c == '#') acceptStatus.insert(nextStatusNum);
				}
				nextStatus.clear();
			}
		}
		statusQueue.pop();
		++curStatusNum;
		addStatusCnt = 1;
	}

	this->statusCnt = statusVec.size();
	FindAccept(statusVec, posName,acceptedToken);
}

DFA& DFA::operator=(DFA&& rhs) {
	if (this != &rhs) {
		statusCnt = rhs.statusCnt;
		acceptStatus = rhs.acceptStatus;
		transitionTable = rhs.transitionTable;
		acceptTokenTable = rhs.acceptTokenTable;
	}
	return *this;
}

DFA::DFA(DFA& src) {
	this->statusCnt = src.statusCnt;
	this->acceptStatus = src.acceptStatus;
	this->acceptTokenTable = src.acceptTokenTable;
	this->transitionTable = src.transitionTable;
}

DFA::DFA(DFA&& src) {
	this->statusCnt = src.statusCnt;
	this->acceptStatus = src.acceptStatus;
	this->acceptTokenTable = src.acceptTokenTable;
	this->transitionTable = src.transitionTable;
}

bool IsBlank(char c) {
	if (c == ' ' || c == '\t' || c == '\n' || c == '\0')
		return true;
	return false;
}

Ty_TokenKind DFA::Recognize(string &word,int &ptr)const {
	ptr = 0;
	char lookAhead;
	int status = 0;
	while (true) {
		status = EdgeTo(status, word[ptr]);
		lookAhead = word[++ptr];
		if (status == -1) return Token::FAILED;
		if (IsAccept(status)) {
			Ty_TokenKind matched = GetAcceptTokenKind(status);
			if (EdgeTo(status, lookAhead) == -1 && !IsBlank(lookAhead))
				return Token::FAILED;
			else if (EdgeTo(status, lookAhead) == -1 && IsBlank(lookAhead))
				return matched;
		}
		else {
			if (EdgeTo(status, lookAhead) == -1)
				return Token::FAILED;
			else continue;
		}
	}
}

bool FiniteAutomata::SetAccpetTokenKind(int acceptStatusNum,Ty_TokenKind tokenKind) {
#ifdef DEBUG
	char buf[256]; 
	_snprintf_s(buf,256,"Status %d is not a accpeted status", acceptStatusNum);
	ASSERT(IsAccept(acceptStatusNum), buf);
#endif // DEBUG
	if (acceptTokenTable.count(acceptStatusNum)) {
		//cerr << "acceptTokenKind has been set" << endl;
		return false;
	}
	else acceptTokenTable[acceptStatusNum] = tokenKind;
	return true;
}

NFA& NFA::operator=(NFA&& rhs) {
	if (this != &rhs) {
		statusCnt = rhs.statusCnt;
		acceptStatus = rhs.acceptStatus;
		transitionTable = rhs.transitionTable;
	}
	return *this;
}

bool NFA::AddEdge(int from, int to, char c) {
#ifdef DEBUG
	ASSERT(from < statusCnt&& to < statusCnt \
		&& c <= 127, "Incorrect status");
#endif // DEBUG
	transitionTable[from][c].insert(to);
}

bool NFA::HasEdge(int from, char c) const {
#ifdef DEBUG
	ASSERT(from < statusCnt&& c <= 127, "Incorrect status");
#endif // DEBUG
	return !transitionTable[from][c].empty();
}

bool NFA::HasEdgeTo(int from, int to, char c)const {
#ifdef DEBUG
	ASSERT(from < statusCnt&& to < statusCnt \
		&& c <= 127, "Incorrect status");
#endif // DEBUG
	return !(transitionTable[from][c].find(to) == transitionTable[from][c].end());
}

const set<int> * NFA::EdgeTo(int from,char c) {
#ifdef DEBUG
	ASSERT(from < statusCnt&& c <= 127, "Incorrect status");
#endif // DEBUG
	return &transitionTable[from][c];
}

void NFA::Print() const{
	for (int i = 0; i < statusCnt; ++i) {
		cout << i << ": ";
		for (int c = 0; c < 128; ++c)
			if (HasEdge(i, c)) {
				cout << "symbol:" << (char)c << "	";
				for (auto& s : transitionTable[i][c])
					cout << s << " ";
				cout << "	";
			}
		cout << "\n";
	}
	cout << "accept status :\n";
	for (auto& i : acceptStatus) cout << "  " << i << "	" << \
		"TokenKind: " << Lexeme::tokenKindNum2Str[GetAcceptTokenKind(i)] << "\n";
	cout << endl;
}

NFA::NFA(vector<DFA>& dfaVec) {
	//NFA的状态数是所有DFA之和+1
	this->statusCnt = 0;
	for (auto& dfa : dfaVec) statusCnt += dfa.GetStatusCnt();
	++statusCnt;

	transitionTable.resize(statusCnt);
	for (auto& s : transitionTable) s.resize(128);

	int offset = 1;
	for (auto& dfa : dfaVec) {
		//NFA初始状态可以空转到DFA初始状态
		AddEdge(0, offset, 0);

		for (int i = 0; i < dfa.GetStatusCnt(); i++) {
			for (int j = 0; j < 128; j++)
				if (dfa.HasEdge(i, j)) {
					AddEdge(i + offset, dfa.EdgeTo(i, j) + offset, j);
				}
		}
		for (auto& accStatus : dfa.GetAcceptStatus()) {
			this->acceptStatus.insert(accStatus + offset);
			this->SetAccpetTokenKind(accStatus + offset, \
				dfa.GetAcceptTokenKind(accStatus));
		}

		offset += dfa.GetStatusCnt();
	}
}

DFA FiniteAutomata::Nfa2Dfa(NFA &nfa) {
	//将第一Dfa状态设置为NFA开始节点的闭包
	set<int> nfaStartStatus;
	nfaStartStatus.insert(0);
	set<int> startStatus = FiniteAutomata::Closure(nfa, nfaStartStatus);
	vector<set<int>> dfaStatus;
	dfaStatus.push_back(startStatus);

	queue<set<int>> q;
	q.push(startStatus);
	set<int> curStatus;
	int curStatusNum = 0;
	set<int> nextStatus;

	DFA dfa;
	dfa.statusCnt = 1;
	dfa.InsertStatus();
	while (!q.empty()) {
		curStatus = q.front();
		q.pop();
		for (int i = 1; i < 128; i++) {

			for (auto &s:curStatus) {
				if (nfa.HasEdge(s, i)) {
					nextStatus.insert(nfa.EdgeTo(s, i)->begin(), \
						nfa.EdgeTo(s, i)->end());
					nextStatus = FiniteAutomata::Closure(nfa, nextStatus);
				}
				if (nfa.IsAccept(s)) {
					dfa.acceptStatus.insert(curStatusNum);
					dfa.SetAccpetTokenKind(curStatusNum, nfa.GetAcceptTokenKind(s));
				}
			}
			//nextStatus不为空，说明对于输出字符i，该状态有出边
			if (!nextStatus.empty()) {
				int sub;
				//如果到达的状态是新状态,更新Dfa数据
				if ((sub = dfa.FindStatus(dfaStatus,nextStatus)) == -1) {
					++dfa.statusCnt;
					dfa.InsertStatus();
					dfaStatus.push_back(nextStatus);
					Ty_Status temp = *(--dfaStatus.end());
					q.push(temp);
					dfa.AddEdge(curStatusNum, dfaStatus.size()-1,i);
				}
				else {
					//否则添加边即可
					dfa.AddEdge(curStatusNum, sub,i);
				}
				nextStatus.clear();
			}
		}
		++curStatusNum;
	};

	return dfa;
}

set<int> FiniteAutomata::Closure(NFA& nfa, set<int>& s) {
	set<int> closureOfS;
	stack <int> st;
	closureOfS.insert(s.begin(),s.end());
	for (auto& i : s) st.push(i);
	while (!st.empty()) {
		auto reached = nfa.EdgeTo(st.top(), 0);
		st.pop();
		if (!reached->empty())
			closureOfS.insert(reached->begin(), reached->end());
	}
	return closureOfS;
}