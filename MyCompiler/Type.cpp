#include "Type.h"
#include "Parser.h"
#include <iostream>
#include <iomanip>
using namespace std;


int Type::maxDefaultTpyeID = 2;
unordered_set<int> Type::arrayTypeSet;
vector<string> Type::typeID2Str = {
	"void","int","float"
};
unordered_map<string, int> Type::typeStr2ID = {
	{"void",0}, {"int",1}, {"float",2}
};

//创建基本数据类型
void Type::BasicTypeDef() {
	typePtr.reserve(20);
	auto voidTy = new Type(0, VOID_WD);
	auto intTy = new Type(1, INT_WD);
	auto floatTy = new Type(2, FLOAT_WD);
	typePtr.push_back(voidTy);
	typePtr.push_back(intTy);
	typePtr.push_back(floatTy);
}

vector<Type *> Type::typePtr;
vector<int> Type::funPara;

int Type::GetTypeWidth(int typeID) {
	return typePtr.at(typeID)->width;
		
}

ArrayType* Type::GetArrayType(int typeID) {
	ArrayType* ap;
	if (ap = dynamic_cast<ArrayType*>( typePtr.at(typeID)) ){
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
	auto a = new ArrayType(elemID,size,typeID);
	string typeStr = GetTypeStr(elemID) + "[]";
	typeStr2ID.insert({ typeStr,typeID });
	typeID2Str.push_back(typeStr);
	typePtr.push_back(a);
	return typeID;
}

string Type::MakeFunTyStr(int retType, vector<int>& funParaDecl) {
	string s = GetTypeStr(retType);
	s += "(";
	for (int i = funParaDecl.size() - 1; i >= 0;--i) {
		s += ( GetTypeStr(funParaDecl[i]) + ",");
	}
	if (s.size() == 1)s += ')';
	else s[s.size() - 1] = ')';
	return s;
}
int Type::FunParaPush(int typeID) {
	funPara.push_back(typeID); 
	return 1; 
}

FunType* Type::GetFunType(int typeID) {
	FunType* fp;
	if (fp = dynamic_cast<FunType*>(typePtr.at(typeID))) {
		return fp;
	}
	else return nullptr;
}

int Type::CreateFunType(int retType) {
	int typeID = typePtr.size();
	auto fp = new FunType(retType,funPara,typeID);
	string typeStr = MakeFunTyStr(retType, funPara);
	typeStr2ID.insert({ typeStr,typeID });
	typeID2Str.push_back(typeStr);
	typePtr.push_back(fp);
	funPara.clear();
	return typeID;
}

int Type::FunCheck(int funTyID){
	FunType * fp;
	if (fp = dynamic_cast<FunType*>(GetTypePtr(funTyID))) {
		for (int i = 0; i < fp->paraType.size(); ++i) {
			if (i >= funPara.size()) {
				cout << "Error : function parameter count doesn't macth \t" \
					<< Parser::RowAndCol();
				exit(1);
			}
			if (funPara[i] != fp->paraType[i]) {
				cout << "Error : function parameter type doesn't macth \t" \
					<< Parser::RowAndCol();
				exit(1);
			}
		}
		return 1;
	}
	else {
		cout << "Error : " << GetTypeStr(funTyID) << " isn't function type \t" << Parser::RowAndCol();
		exit(1);
		return 0;
	}
}

int Type::RetCheck(int t1, int t2) {
	if (t1 != t2) {
		cout << "Error : Function return type error \n" << Parser::RowAndCol();
		exit(1);
	}
	return 1;
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
	if (typeID == VOID_ID) {
		cout << "Error : Cann't create a variable which is void type : " << lexeme << " " << Parser::RowAndCol() << endl;
		exit(1);
		return false;
	}
	auto v = new Variable(typeID, lexeme, offset + base);
	symTable.insert({ lexeme, v });
	offset += v->t->width;
	return true;
}

Variable* Environment::EnvGet(string &lexeme) {
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
	Type* t = curEnv->EnvGet(lex)->t;
	auto ap = Type::GetArrayType(t->typeID);
	return ap;
}

int Environment::SetFunStart(string lex,int codeStart) {
	auto f = Environment::curEnv->EnvGet(lex);
	if (f->t->IsFunType()) f->addr = codeStart;
	else {
		cout << "Error : " << lex << " isn't a function \n" << Parser::RowAndCol();
		exit(1);
	}
}

int Environment::GetRetType(string lex) {
	auto f = Environment::curEnv->EnvGet(lex);
	FunType* ft;
	if (ft = dynamic_cast<FunType*>(f->t)) {
		return ft->retType;
	}
	else {
		cout << "Error : " << lex << " isn't a function \t" << Parser::RowAndCol();
		exit(1);
	}
	return 1;
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
	cout << setiosflags(ios::left);
	cout << setw(20) <<"lexeme" << setw(20) << "type" << setw(20) << "taddr" << setw(20) << "width" << "\n";
	for (auto& i : symTable) {
		cout <<setw(20) << i.first << setw(20) << Type::GetTypeStr(i.second->t->typeID) << \
			 hex << "0x" << setw(18) <<i.second->addr << dec << setw(20) << i.second->t->width << "\n";
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
	Type::ReleaseTypePtr();
}

ArrayType::ArrayType(int elemID, int size,int arrayTypeID) : elemID(elemID),size(size){
	elemWidth = Type::GetTypeWidth(elemID);
	this->typeID = arrayTypeID;
	this->width = size * elemWidth;
}

FunType::FunType(int retType, vector<int>& funParaDecl,int funTyID) {
	this->typeID = funTyID;
	this->width = FUN_TYWD;
	this->retType = retType;
	for (int i = funParaDecl.size() - 1; i >= 0; --i)
		paraType.push_back(funParaDecl[i]);
}