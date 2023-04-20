#include "FiniteAutomata.h"
#include <queue>

bool DFA::IsAccept(int status) {
#ifdef DEBUG
	ASSERT(status <= this->statusCnt, "Incorrect status");
#endif // DEBUG
	return acceptStatus.find(status) == acceptStatus.end();
}

void DFA::Print() {
	for (int i = 0; i < this->statusCnt; ++i) {
		cout << i << ":" << "	";
		for (int j = 0; j < 128; j++) {
			if (EdgeTo(i, j) != -1) cout << "symbol: " << j << " " \
				<< EdgeTo(i, j) << "	";
		}
		cout << "\n";
	}
	cout << "accept status :";
	for (auto& i : acceptStatus) cout << i <<" ";
	cout << endl;
}

bool DFA::HasEdge(int from, char symbol) {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127, "Incorrect status");
#endif // DEBUG
	if (transitionTable[from][symbol] == -1) return false;
	return true;
}

bool DFA::HasEdgeTo(int from, int to, char symbol) {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127 && to <= statusCnt, "Incorrect status");
#endif // DEBUG
	if (transitionTable[from][symbol] == to) return true;
	return false;
}

int DFA::EdgeTo(int from, char symbol) {
#ifdef DEBUG
	ASSERT(from <= statusCnt && symbol <= 127, "Incorrect status");
#endif // DEBUG
	return transitionTable[from][symbol];
}

int DFA::FindStatus(vector<Ty_Status>& statusVec,Ty_Status &s) {
	size_t size = statusVec.size();
	for (size_t i = 0; i < size; i++)
		if (statusVec[i] == s) return i;
	return -1;
}

bool DFA::AddEdge(int from, int to, char symbol) {
	if (transitionTable[from][symbol] == -1) {
		transitionTable[from][symbol] = to;
		return true;
	}
	return false;
}

void DFA::InsertStatus() {
	transitionTable.push_back(new int[128]);
	auto line = *(--transitionTable.end());
	for (int i = 0; i < 128; i++) line[i] = -1;
}

void DFA::FindAccept(vector<Ty_Status>& statusVec,vector<int>& posName) {
	int size = statusVec.size();
	for (int i = 0; i < size; i++)
	{
		for (auto& pos : statusVec[i])
			if (posName[pos] == '#') acceptStatus.insert(i);
	}
}

DFA::DFA(SyntalTreePtr tree,Ty_FollowPos &followPos) {
	int id = tree->GetID();
	auto root = tree->GetRoot();

	//每个pos对应的char都可以用下标索引到
	auto posName = SyntalTree::leftNodeTable[id];

	cout << *followPos[4].cbegin() << " " << (char)posName[*followPos[4].cbegin()] << "\n";

	queue<set<int>> statusQueue;
	vector<set<int>> statusVec;
	int curStatusNum = 0;
	statusQueue.push(root->firstPos);
	statusVec.push_back(root->firstPos);

	InsertStatus();
	set<int>* curStatus;
	set<int> nextStatus;

	while (!statusQueue.empty()){
		curStatus = &statusQueue.front();
		//对于每一个输入
		int c;
		for (c = 0; c < 128; ++c) {
			for (auto &pos: *curStatus) {
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
					nextStatusNum = curStatusNum + 1;
					AddEdge(curStatusNum, nextStatusNum, c);

					if (c == '#') acceptStatus.insert(nextStatusNum);
				}
				nextStatus.clear();
			}
		}
		statusQueue.pop();
		++curStatusNum;
	}

	this->statusCnt = statusVec.size();
	FindAccept(statusVec,posName);
}