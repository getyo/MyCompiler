#include "Type.h"
#include "Parser.h"
#include <iostream>
using namespace std;


int Type::maxDefaultTpyeID = 1;
unordered_set<int> Type::arrayTypeSet;
vector<string> Type::typeID2Str = {
	"int","float"
};
unordered_map<string, int> Type::typeStr2ID = {
	{"int",0}, {"float",1}
};


//创建基本数据类型
void Type::BasicTypeDef() {
	typePtr.reserve(20);
	Type* intTy = new Type(0, 4);
	Type* floatTy = new Type(1, 4);
	typePtr.push_back(intTy);
	typePtr.push_back(floatTy);
}

vector<Type*> Type::typePtr;

int Type::GetTypeWidth(int typeID) {
	return typePtr.at(typeID)->width;
		
}

ArrayType* Type::GetArrayType(int typeID) {
	ArrayType* ap;
	if (ap = dynamic_cast<ArrayType*>(typePtr.at(typeID))){
		return ap;
	}
	else {
		cout << "Error : The array dimension does not match the definition \t" << Parser::RowAndCol() << "\n";
		exit(1);
		return nullptr;
	}
}



int Type::CreateArrayType(int elemID, int size) {
	int typeID = typePtr.size();
	ArrayType* a = new ArrayType(elemID,size,typeID);
	string typeStr = GetTypeStr(elemID) + "[]";
	typeStr2ID.insert({ typeStr,typeID });
	typeID2Str.push_back(typeStr);
	typePtr.push_back(a);
	return typeID;
}


Environment* Environment::curEnv = nullptr;
size_t Environment::dataFieldSize = DATA_START;
vector<Environment*> Environment::envAll;
static size_t EnvID = 0;

size_t Environment::getID() {
	return EnvID++;
}


Environment::Environment(Environment * pre){
	if (pre == nullptr) {
		base = dataFieldSize;
		this->envID = Environment::getID();
		offset = 0;
		this->pre = nullptr;
		Type::BasicTypeDef();
		envAll.push_back(this);
		return;
	}
	this->base = dataFieldSize;
	this->envID = Environment::getID();
	offset = 0;
	this->pre = pre;
	envAll.push_back(this);
}

Environment* Environment::NewEnv() {
	if (curEnv == nullptr) {
		curEnv = new Environment(nullptr);
		return curEnv;
	}
		
	Environment* env = new Environment(curEnv);
	curEnv = env;
	return env;
}

Environment* Environment::PopEnv() {
	auto temp = curEnv;
	curEnv = curEnv->pre;
	return curEnv;
}

bool Environment::EnvPush(string &lexeme, int typeID) {
	if (symTable.count(lexeme)) {
		cout << "Error : Redefination of identifier : " << lexeme << " " << Parser::RowAndCol() << endl;
		exit(1);
		return false;
	}
	symTable.insert({ lexeme,Variable(typeID,lexeme,offset + base) });
	offset += symTable[lexeme].t.width;
}

Variable& Environment::EnvGet(string &lexeme) {
	if (!symTable.count(lexeme)) {
		if (pre == nullptr) {
			cout << "Error : No identifier defined : " << lexeme << " " << Parser::RowAndCol() << endl;
			exit(1);
		}
		else return pre->EnvGet(lexeme);
	}
	return symTable.at(lexeme);
}

ArrayType* Environment::GetArrayType(string lex) {
	Type t = curEnv->EnvGet(lex).t;
	auto ap = Type::GetArrayType(t.typeID);
	return ap;
}

void Environment::PrintCur() {
	Environment* te = curEnv;
	while (te != nullptr) {
		te->Print();
		te = curEnv->pre;
	}
}

void Environment::Print() {
	cout << "Env" << envID << ": \n";
	cout << " \tlexeme \t\ttype \t\taddr \t\twidth\n";
	for (auto& i : symTable) {
		cout << " \t" << i.first << " \t\t" << Type::GetTypeStr(i.second.t.typeID) << \
			" \t\t" << hex << "0x" << i.second.addr << dec << " \t\t" << i.second.t.width << "\n";
	}
}

void Environment::PrintAll() {
	for (auto& e : envAll) {
		cout << "\n";
		e->Print();
	}
}

void Environment::DeleteAll() {
	for (auto& e : envAll)
		delete e;
}

ArrayType::ArrayType(int elemID, int size,int arrayTypeID) : elemID(elemID),size(size){
	elemWidth = Type::GetTypeWidth(elemID);
	this->typeID = arrayTypeID;
	this->width = size * elemWidth;
}


