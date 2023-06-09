#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "Reg.h"
using namespace std;

struct ArrayType;
class Environment;
struct Type;
struct FunType;
#define FUN_TYWD -1
#define VOID_ID 0
#define VOID_WD 0
#define INT_WD 4
#define FLOAT_WD 4
struct Type
{
private:
	static unordered_set<int> arrayTypeSet;
	static vector<string> typeID2Str;
	static unordered_map<string,int> typeStr2ID;
	static vector<Type*> typePtr;
	static int maxDefaultTpyeID;
	static vector<int> funPara;

	static string MakeFunTyStr(int retType, vector<int>& funPara);
public:
	int typeID;
	//如果为0代表类型为void,为-1代表类型为函数
	int width;
	static void BasicTypeDef();
	static Type* GetTypePtr(int typeID) { return typePtr.at(typeID); }
	static int GetTypeID(string s) { return typeStr2ID.at(s); }
	static int GetTypeWidth(int typeID);
	static int GetTypeWidth(string typeName) { return GetTypeWidth(typeStr2ID.at(typeName)); }
	static string GetTypeStr(int typeID) { return typeID2Str.at(typeID); }
	Type(){}
	Type(int typeID,int w):typeID(typeID),width(w){}
	Type(int typeID) : typeID(typeID), width(GetTypeWidth(typeID)) {};
	Type(string typeName) : typeID(GetTypeID(typeName)), width(GetTypeWidth(typeName)) {};
	Type(Type& t) :typeID(t.typeID), width(t.width) {};
	Type(Type&& t) :typeID(t.typeID), width(t.width) {};
	Type& operator=(const Type& t) {
		this->typeID = t.typeID;
		this->width = t.width;
		return *this;
	}
	Type& operator=(const Type&& t) {
		this->typeID = t.typeID;
		this->width = t.width;
		return *this;
	}
	bool IsFunType() { return width == FUN_TYWD; }

	static ArrayType* GetArrayType(int typeID);
	static int CreateArrayType(int elemID, int size);

	static int FunParaPush(int typeID);
	static FunType * GetFunType(int typeID);
	static int CreateFunType(int retType);

	static void ClearFunPara() { funPara.clear(); }
	//类型检测以及转化
	static int FunCheck(int funTyID);
	static int RetCheck(int t1, int t2);
	virtual ~Type(){}
	static void ReleaseTypePtr() {
		for (auto& t : typePtr)
			delete t;
	}
};


//也可以表示函数
struct Variable {
	static const int UNUSED;
	Type * t;
	string name;
	Environment * e;
	//如果是函数则表示第一条指令的起始地址
	int addr;
	//寄存器编号，详见Reg.h中的规定
	int reg = REG_EMPTY;
	bool live = true;
	int lastUsed = UNUSED;
	Variable* belong = nullptr;
	Environment* funEnv = nullptr;
	Variable(){}
	Variable(int typeID, string name, int addr) :t( Type::GetTypePtr(typeID) ), name(name), addr(addr) {}
	Variable(string typeName, string name, int addr) : Variable(Type::GetTypeID(typeName),name,addr) {}
};

#define DATA_START 0

class Environment {
	friend class Variable;
private:
	static size_t getID();
	static vector<Environment*> envAll;
	static int* localSizePtr;
	size_t envID;
	unsigned int base;
	unsigned int offset = 0;
	int localVarSize = 0;
	unordered_map<string,Variable *> symTable;
	Environment* pre; 
	Environment(Environment* pre);
public:
	int size() { 
		if (localVarSize) return localVarSize;
		else return offset;
	}
	const unordered_map<string, Variable*> & GetSymTable() const { return this->symTable; }
	static Environment* Global(){ return envAll[0]; }
	static size_t dataFieldSize;
	static Environment* curEnv;
	Variable* EnvGet(string &lexeme);
	bool EnvPush(string &lexeme, int typeID);
	ArrayType* GetArrayType(string lex);
	Environment* GetPre() { return pre; }
	int GetRetType(string lex);
	static void SetFunLocalVar() { Environment::localSizePtr = &curEnv->localVarSize; }
	static Environment* NewEnv();
	static Environment* PopEnv();
	static int NewTemp(int typeID) { 
		dataFieldSize += Type::GetTypeWidth(typeID); 
		return -1; 
	}
	static int SetFunStart(string lex,int codeStart);
	static void PrintAll();
	void PrintCur();
	void Print();
	static void DeleteAll();
};

struct ArrayType :Type{
	//不是数组类型的ID，而是数组里面元素的ID
	int elemID;
	int elemWidth;
	int size;

	ArrayType(){}
	ArrayType(int elemID, int size,int arrayTypeID);
};

struct FunType :Type {
	int retType;
	vector <int> paraType;

	FunType();
	FunType(int retType,vector<int>& funPara,int funTyID);
};