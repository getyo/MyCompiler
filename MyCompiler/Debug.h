#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#include <ios>
#include <fstream>
#include <functional>

//#define DEBUG 
#ifdef DEBUG
extern std::ofstream logOut;
void PrintAndOutputToLog(std::function<void()>);
void PrintStackTrace();
#define STACK_INFO_LEN  1024
#define MAX_STACK_FRAMES 12
#define ASSERT(expr,info) {if(!(expr)) {\
		cerr << "\n" << info << "\n";\
		PrintStackTrace();\
		abort();\
	}}


#define _PARSER
	#ifdef _PARSER
		//#define _GRAMMER_PRINT
		//#define _LR0_PRINT
		//#define _INIT_LOOKAHEAD_PRINT
		//#define _COLLECTION_PRINT
		#define _PARSER_REDUCE_PRINT
		#define _PARSER_SHIFT_PRINT
		//#define _PARSER_STACK_PRINT
	#endif // _PARSER
#define _GEN
	#ifdef _GEN
		#define _ICP
		#define _ENVP	
	#endif // _GEN
#else
#define ASSERT(expr,info) ;
#endif // DEBUG

