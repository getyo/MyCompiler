#include "Parser.h"
#include <stack>
Parser* Parser::parserPtr = nullptr;
extern SymbolTable symbolTable;

Parser::Parser() {
	collectionPtr = Collection::CollectionFactory();
	grammer = collectionPtr->GetGrammer();
	nonInt = grammer->grammerSymbolStr2Num["_NON"];
	grammer->Print();
	collectionPtr->Print();
}

Parser::~Parser() {
	if (parserPtr != nullptr) {
		delete collectionPtr;
	}
}

void Parser::SetInput(vector <Token>& tokenStream) {
	this->tokenStream = &tokenStream;
}

string Parser::makeErrorInfo(Token t) {
	int index = t.symbolTableIndex;
	string s = "Syntal Error: line " + to_string(symbolTable[index].row) \
				+ " column " + to_string(symbolTable[index].row) \
				+ " illegal word : " + symbolTable[index].lexeme + "\n";
	return s;
}

void Parser::PopToken(int cnt) {
	for (int i = 0; i < cnt; i++) {
		symbolStack.pop();
	}
}

int Parser::Reduce(int productionIndex) {
	Production &p = (*grammer)[productionIndex];
	auto body = p.GetBody();
	int topTokenKind;
	for (int i = body.size() - 1; i >= 0; i--) {
		topTokenKind = symbolStack.top();
		if (topTokenKind == body[i]) {
			PopToken(1);
			statusStack.pop();
		}
		else 
			return -1;
	}
	curStatus = statusStack.top();
	return p.GetHead();
}

void Parser::shift(int symbol){
	symbolStack.push(symbol);
	curStatus = collectionPtr->Goto(curStatus, symbol);
	statusStack.push(curStatus);
}

bool Parser::RedressNon() {
	curInputToken = nonInt;
	action = collectionPtr->Goto(curStatus, curInputToken);
	if (action == Collection::NON_ENTRY) return false;
	else if (action >= 0) {
		shift(curInputToken);
		curInputToken = (*tokenStream)[tokenIndex].kind;
		return true;
	}
	else if(action < 0) {
		//注意 在PraserTable中-1代表使用Production0进行归约
		curInputToken = Reduce(-(action + 1));
		(*grammer)[-(action + 1)].Print();
		shift(curInputToken);
		curInputToken = (*tokenStream)[tokenIndex].kind;
		cout << "\n";
		return true;
	}
	return false;
}

bool Parser::Analyse() {
	bool success = true;
	curStatus = 0;
	statusStack.push(curStatus);
	curInputToken = (*tokenStream)[tokenIndex].kind;
	action = collectionPtr->Goto(curStatus, curInputToken);
	
	while(tokenIndex < tokenStream->size()){
		if (action == Collection::NON_ENTRY) {
			bool redressNon = RedressNon();
			if (action == Collection::ACCESS) return true;
			if (redressNon) goto Continue;
			errorInfo.push_back(makeErrorInfo((*tokenStream)[tokenIndex]));
			return false;
		}
		//shift操作
		else if (action >= 0) {
			shift(curInputToken);
			++tokenIndex;
			curInputToken = (*tokenStream)[tokenIndex].kind;
		}
		//reduce操作
		else {
			//注意 在PraserTable中-1代表使用Production0进行归约
			curInputToken = Reduce(-(action + 1));
			(*grammer)[-(action + 1)].Print();
			if (action == Collection::ACCESS) return true;
			shift(curInputToken);
			curInputToken = (*tokenStream)[tokenIndex].kind;
			cout << "\n";
		}
Continue:
		action = collectionPtr->Goto(curStatus, curInputToken);
	}
}

void Parser::PrintError() {
	for (auto& s : errorInfo)
		cout << s;
}

Parser* Parser::ParserFactory() {
	if (parserPtr == nullptr)
		parserPtr = new Parser();
	return parserPtr;
}