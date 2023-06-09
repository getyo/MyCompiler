#pragma once
#include "Type.h"
#include "Triple.h"
#include <iostream>
#include "Parser.h"
using namespace std;

#define STATTR_2_STR(attrPtr) ( *((string*)attrPtr) )
#define INTATTR_2_INT(attrPtr) ( *((int*)attrPtr) )
#define PTR2INT(voidPtr )( (int)voidPtr)
#define INT_ATTR(index) INTATTR_2_INT(paraList[index])
#define INT_CONST(index) PTR2INT(paraList[index])
#define ST_ATTR(index) STATTR_2_STR(paraList[index])

#define FUNCALL(index, paraList,retVal) {\
	switch (index){\
		case 0:{retVal = (int)Environment::NewEnv();break;}\
		case 1:{retVal = (int)Environment::PopEnv();break;}\
		case 2:{retVal = (int)Environment::NewTemp(INT_CONST(0)); break;}\
		case 3:{retVal = (int)Environment::curEnv->EnvPush( ST_ATTR(0) ,INT_ATTR(1) );break;}\
		case 4:{retVal = (int)Environment::curEnv->EnvGet( ST_ATTR(0) )->addr; break;}\
		case 5:{retVal = (int)Environment::curEnv->EnvGet( ST_ATTR(0) )->t->typeID; break;}\
		case 6:{retVal = (int)Environment::curEnv->EnvGet( ST_ATTR(0) )->t->width; break;}\
		case 7:{retVal = (int)Environment::curEnv->EnvGet( ST_ATTR(0) ) ; break;}\
		case 8:{retVal = (int)Generator::Gen( INT_CONST(0) , INT_ATTR(1) , INT_ATTR(2) ); break;}\
		case 9:{retVal = (int)Generator::Update(); break;}\
		case 10:{retVal = (int)Generator::InsertElem( INT_ATTR(0) , INT_ATTR(1) ); break;}\
		case 11:{retVal = (int)Type::CreateArrayType( INT_ATTR(0) , INT_ATTR(1) ); break;}\
		case 12:{retVal = (int)Type::GetArrayType( INT_ATTR(0) )->elemWidth; break;}\
		case 13:{retVal = (int)Type::GetArrayType( INT_ATTR(0) )->elemID; break;}\
		case 14:{retVal = (int)Type::GetTypeWidth( INT_ATTR(0) ) ; break;}\
		case 15:{retVal = (int)Generator::BackPatch( INT_ATTR(0) , INT_ATTR(1) ) ; break;}\
		case 16:{retVal = (int)Generator::DoPatch( INT_ATTR(0) ) ; break;}\
		case 17:{retVal = (int)Type::FunParaPush( INT_ATTR(0) ) ; break;}\
		case 18:{retVal = (int)Type::CreateFunType( INT_ATTR(0) ) ; break;}\
		case 19:{retVal = (int)Type::FunParaPush( INT_ATTR(0) ) ; break;}\
		case 20:{retVal = (int)Type::FunCheck( INT_ATTR(0) ) ; break;}\
		case 21:{retVal = (int)Generator::GetCodeStart() ; break;}\
		case 22:{retVal = (int)paraList[0]; break;}\
		case 23:{retVal = (int)Environment::SetFunStart( ST_ATTR(0) , INT_ATTR(1) ); break;}\
		case 24:{retVal = Type::RetCheck( INT_ATTR(0), INT_ATTR(1) ); break;}\
		case 25:{retVal = Environment::curEnv->GetRetType( ST_ATTR(0) ); break;}\
		case 26:{retVal = Environment::curEnv->GetPre()->EnvPush( ST_ATTR(0) ,INT_ATTR(1) ); break;}\
		case 27:{retVal = (int)Environment::curEnv; break;}\
		case 28:{retVal = (int)Environment::curEnv->GetPre()->EnvGet( ST_ATTR(0) ); break;}\
		case 29:{retVal = Generator::Label(); break;}\
		default:{\
			cerr << "Parser: illegal function ID"; abort();break;\
		}\
	};\
	}

int PrintSymbolAttr(int,int*);
