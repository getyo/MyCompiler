#pragma once
#define FUNCALL(index, ...) ({\
	int retVal;\
	switch (index){\
		case 0:{retVal = PrintSymbolAttr(__VA_ARGS__)}\
	}retVal;})

int PrintSymbolAttr(int,int*);
