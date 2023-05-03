#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#define DEBUG 
#ifdef DEBUG
#define _PARSER
	#ifdef _PARSER
		//#define _GRAMMER_PRINT
		//#define _LR0_PRINT
		//#define _COLLECTION_PRINT
		#define _PARSER_ACTION_PRINT
		//#define _PARSER_STACK_PRINT
	#endif // _PARSER


#endif // DEBUG


void PrintStackTrace();
#define ASSERT(expr,info) {if(!(expr)) {\
		cerr << "\n" << info << "\n";\
		PrintStackTrace();\
		abort();\
	}}

#define STACK_INFO_LEN  1024
#define MAX_STACK_FRAMES 12