#pragma once
#include "Type.h"
#include "Triple.h"

#define STATTR_2_STR(attrPtr) ( *((string*)attrPtr) )
#define INTATTR_2_INT(attrPtr) ( *((int*)attrPtr) )
#define PTR2INT(voidPtr )( (int)voidPtr)
#define INT_ATTR(index) INTATTR_2_INT(paraList[index])
#define INT_CONST(index) PTR2INT(paraList[index])
#define ST_ATTR(index) STATTR_2_STR(paraList[index])

#define FUNCALL(index, paraList,retVal) {\
	switch (index){\
		case 0:{retVal = (int)Environmemt::NewEnv();break;}\
		case 1:{retVal = (int)Environmemt::PopEnv();break;}\
		case 2:{retVal = (int)Environmemt::NewTemp(INT_CONST(0)); break;}\
		case 3:{retVal = (int)Environmemt::curEnv->EnvPush( ST_ATTR(0) ,INT_ATTR(1) );break;}\
		case 4:{retVal = (int)Environmemt::curEnv->EnvGet( ST_ATTR(0) ).addr; break;}\
		case 5:{retVal = (int)Environmemt::curEnv->EnvGet( ST_ATTR(0) ).t.typeID; break;}\
		case 6:{retVal = (int)Environmemt::curEnv->EnvGet( ST_ATTR(0) ).t.width; break;}\
		case 7:{retVal = (int)Generator::Gen( INT_CONST(0) , INT_ATTR(1) , INT_ATTR(2) ); break;}\
		case 8:{retVal = (int)Generator::Update(); break;}\
		case 9:{retVal = (int)Generator::InsertElem( INT_ATTR(0) , INT_ATTR(1) ); break;}\
		case 10:{retVal = (int)Generator::GenAssign( INT_CONST(0) , INT_ATTR(1) ,INT_ATTR(2) ); break;}\
		default:{\
			cerr << "Parser: illegal function ID"; break;\
		}\
	};\
	}

int PrintSymbolAttr(int,int*);