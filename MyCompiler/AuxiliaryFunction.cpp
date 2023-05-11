#include "AuxiliaryFunction.h"
#include <vector>
#include <iostream>
#include "Grammer.h"
#include "Production.h"
using namespace std;

int PrintSymbolAttr(int symbolID,int * attribute) {
	cout << Grammer::GetSymbolStr(symbolID) << " : ";
	for (int i = 0; i < SYMBOL_ATTR_CNT; i++) {
		if (attribute[i] != -1) cout << attribute[i] << " ";
		else break;
	}
	cout << endl;
	return -1;
}