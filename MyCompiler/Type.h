#pragma once
#include <vector>
#include <unordered_map>
using namespace std;


struct Type
{private:
	static vector<string> typeID2Str;
	static unordered_map<string,int> typeStr2ID;
	static vector<int> typeWidth;
public:
	int typeID;
	size_t width;
	static int GetTypeID(string s) { return typeStr2ID.at(s); }
	static int GetTypeWidth(int typeID) { return typeWidth.at(typeID); }
	static int GetTypeWidth(string typeName) { return typeWidth.at(typeStr2ID.at(typeName)); }
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
	static Environmemt* NewEnv();
	static Environmemt* PopEnv();
	static int NewTemp(int typeID) { 
		dataFieldSize += Type::GetTypeWidth(typeID); 
		return -1; 
	}
};