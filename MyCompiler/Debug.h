#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#include <ios>
#include <fstream>
#include <functional>

#define DEBUG 
extern std::ofstream logOut;
#ifdef DEBUG
#define _PARSER
	#ifdef _PARSER
		#define _GRAMMER_PRINT
		//#define _LR0_PRINT
		//#define _INIT_LOOKAHEAD_PRINT
		//#define _COLLECTION_PRINT
		#define _PARSER_ACTION_PRINT
		//#define _PARSER_STACK_PRINT
	#endif // _PARSER
#define _GEN
	#ifdef _GEN
		#define _ICP
	#endif // _GEN

#endif // DEBUG

void PrintAndOutputToLog(std::function<void()>);
void PrintStackTrace();
#define ASSERT(expr,info) {if(!(expr)) {\
		cerr << "\n" << info << "\n";\
		PrintStackTrace();\
		abort();\
	}}

#define STACK_INFO_LEN  1024
#define MAX_STACK_FRAMES 12