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
	//����Env������ջ�ռ�
	void FunStart(Variable * f);
	void FunEnd();
	void GenJc(Triple &ic);
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