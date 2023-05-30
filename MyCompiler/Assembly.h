#pragma once
#include "Triple.h"
#include "Type.h"
#include "Reg.h"

class AssGen {
	friend class RegManager;
private:
	
	CodeStore* ic;
	AssemblyStore* as;
	RegManager* regMan;
	static AssGen* assGenPtr;
	static int stBase;
	static bool beforeGen;

	struct Block {
		int start;
		int end;
		Block* next = nullptr;
		Block* jmp = nullptr;
		Block(){}
		Block(int start, int end) :start(start), end(end){}
	};
	vector <Block> blocks;

	void GlobalData();
	bool IsLeader(Triple& t);
	void PartBlock();
	void Gen(Block& b);
	//根据Env来开辟栈空间
	void FunStart(Variable * f);
	void FunEnd(Triple &t);
	void GenJc(Triple &ic);
	void GenCall(Triple& t);
	AssGen(AssemblyStore* as) : as(as) {}
	~AssGen() {
		RegManager::Release();
	}
public:
	static AssGen* AssGenFactory(AssemblyStore& as);
	void SetInput(CodeStore& ic);
	void Gen();
	void Print();
	static void Release() {
		if (assGenPtr != nullptr) {
			delete assGenPtr;
			assGenPtr = nullptr;
		}
	}
};