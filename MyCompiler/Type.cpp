#include "Type.h"
#include "Parser.h"
#include <iostream>
using namespace std;

vector<string> Type::typeID2Str = {
	"int","float"
};
unordered_map<string, int> Type::typeStr2ID = {
	{"int",0}, {"float",1}
};
vector<int> Type::typeWidth = {
	4,4
};

Environmemt* Environmemt::curEnv = nullptr;
size_t Environmemt::dataFieldSize = 0;

static size_t EnvID = 0;

size_t Environmemt::getID() {
	return EnvID++;
}

Environmemt::Environmemt(Environmemt * pre){
	if (pre == nullptr) {
		base = 0;
		offset = 0;
		this->pre = nullptr;
		return;
	}
	this->base = pre->offset;
	offset = 0;
	this->pre = pre;
}

Environmemt* Environmemt::NewEnv() {
	if (curEnv == nullptr) {
		curEnv = new Environmemt(nullptr);
		return curEnv;
	}
		
	Environmemt* env = new Environmemt(curEnv);
	curEnv = env;
	return env;
}

Environmemt* Environmemt::PopEnv() {
	auto temp = curEnv;
	curEnv = curEnv->pre;
	delete temp;
	return curEnv;
}

bool Environmemt::EnvPush(string lexeme, int typeID) {
	if (symTable.count(lexeme)) {
		cout << "Redefination of identifier : " << lexeme << " " << Parser::RowAndCol() << endl;
		return false;
	}
	symTable.insert({ lexeme,Variable(typeID,lexeme,offset) });
	offset += symTable[lexeme].t.width;
}

Variable& Environmemt::EnvGet(string lexeme) {
	if (!symTable.count(lexeme)) {
		cout << "No identifier defined : " << lexeme <<  " " << Parser::RowAndCol() << endl;
		exit(1);
	}
	return symTable.at(lexeme);
}