#pragma once
#define DEBUG 
#include <iostream>
#include <assert.h>
#define ASSERT(expr,info) {if(!(expr)) {\
		cerr << info;\
		assert(false);\
	}}