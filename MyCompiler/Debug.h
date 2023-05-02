#pragma once
#define DEBUG 
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#define _GRAMMER_PRINT
#define _LR0_PRINT

void PrintStackTrace();
#define ASSERT(expr,info) {if(!(expr)) {\
		cerr << "\n" << info << "\n";\
		PrintStackTrace();\
		abort();\
	}}

#define STACK_INFO_LEN  1024
#define MAX_STACK_FRAMES 12