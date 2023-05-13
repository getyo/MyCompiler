#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
using namespace std;

class ArrayType;

union WidthOrPtr
{
	int width;
	void* typeDef;
	WidthOrPtr(){}
	WidthOrPtr(int width) :width(width) {}
	WidthOrPtr (void * ptr):typeDef(ptr){}
};

struct Type
{private:
	static unordered_set<int> arrayTypeSet;
	static vector<string> typeID2Str;
	static unordered_map<string,int> typeStr2ID;
	static vector<WidthOrPtr> typeWidth;
	static int maxDefaultTpyeID;
	static string CreateArrayStr(int typeID, int dim);

public:
	int typeID;
	size_t width;
	static int GetTypeID(string s) { return typeStr2ID.at(s); }
	static int GetTypeWidth(int typeID);
	static ArrayType* GetArrayType(int typeID);
	static int GetTypeWidth(string typeName) { return GetTypeWidth(typeStr2ID.at(typeName)); }
	static string GetTypeStr(int typeID) { return typeID2Str.at(typeID); }
	Type(){}
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
	static int CreateArrayType(int typeID, int dim1Size, int dim2Size, int dim3Size);
};

struct Variable {
	Type t;
	string name;
	unsigned int addr;
	Variable(){}
	Variable(int typeID, string name, int addr) :t(Type(typeID)), \
		name(name), addr(addr) {};
	Variable(string typeName, string name, int addr) :t(Type(typeName)), \
		name(name), addr(addr) {};
};

class Environmemt {
private:
	static size_t dataFieldSize;
	static size_t getID();
	size_t envID;
	unsigned int base;
	unsigned int offset;
	unordered_map<string,Variable> symTable;
	Environmemt* pre;
	Environmemt(Environmemt* pre);
public:
	static Environmemt* curEnv;
	Variable& EnvGet(string lexeme);
	bool EnvPush(string lexeme, int typeID);
	ArrayType* GetArrayType(string lex);
	static Environmemt* NewEnv();
	static Environmemt* PopEnv();
	static int NewTemp(int typeID) { 
		dataFieldSize += Type::GetTypeWidth(typeID); 
		return -1; 
	}
	static void Print();
};

struct ArrayType {
	//不是数组类型的ID，而是数组里面元素的ID
	int elemID;
	int elemWidth;
	int dim;
	int rowSize1;
	int rowSize2;
	int rowSize3;
	int totalSize;
	ArrayType(){}
	ArrayType(int elemID, int rowSize1, int rowSize2 = -1, int rowSize3 = -1) :\
		elemID(elemID), rowSize1(rowSize1), rowSize2(rowSize2), rowSize3(rowSize3) {
		elemWidth = Type::GetTypeWidth(elemID);
		totalSize = rowSize1 * elemWidth;
		dim = 1;
		if (rowSize2 > 0) {
			totalSize *= rowSize2;
			dim += 1;
		}
		if (rowSize3 > 0) {
			totalSize *= rowSize3;
			dim += 1;
		}
	}
	int GetDimSize(int dim);
};