#include "Parser.h"
#include <stack>
#include "Debug.h"
Parser* Parser::parserPtr = nullptr;
extern SymbolTable symbolTable;

Parser::Parser() {
	collectionPtr = Collection::CollectionFactory();
	grammer = collectionPtr->GetGrammer();
	nonInt = Grammer::GetSymbolID("_NON");
}

void Parser::SetInput(vector <Token>& tokenStream) {
	this->tokenStream = &tokenStream;
}

string Parser::makeErrorInfo(Token t) {
	int index = t.symbolTableIndex;
	string s = "Syntal Error: line " + to_string(symbolTable[index].row) \
				+ " column " + to_string(symbolTable[index].col) \
				+ " illegal word : " + symbolTable[index].lexeme + "\n";
	return s;
}

//使用该函数时默认符号栈以及含有该表达式全部内容
void* Parser::GetAttrPtr(int pItr, int smbIndex, int attrIndex) {
	auto& p = (*grammer)[pItr];
	int bodysize = p.GetBody().size();
	int symID = p.GetBody()[smbIndex - 1];
	int offset = smbIndex - bodysize;
	auto& swa = symbolStack.get(offset);

	if (Grammer::IsTerminal(symID)) {
		auto& unterminalAttr  = symbolTable[swa.symTableIndex];
		switch (attrIndex)
		{
		case 0: {return &unterminalAttr.lexeme; }
		case 1: {return &unterminalAttr.val; }
		case 2: {return &unterminalAttr.row; }
		case 3: {return &unterminalAttr.col; }
		case 4: {return &unterminalAttr.typeID; }
		default: {
			cerr << "Parser : Incorrect Unterminal Attribute index : " + attrIndex << "\n";
			PrintStackTrace();
			abort();
			break;
		}
		}
	}
	else {
		return &swa.attr->at(attrIndex);
	}
}

void ComputedOp(stack<void*>& oprandStack,vector<int*>&unused,int op) {
	int oprand1 = *(int*)oprandStack.top();
	oprandStack.pop();
	int oprand2 = *(int*)oprandStack.top();
	oprandStack.pop();
	int* res = new int();
	switch (op)
	{
	case Action::ADD: {*res = oprand1 + oprand2; break; }
	case Action::MINUS: {*res = oprand1 - oprand2; break; }
	case Action::MULT: {*res = oprand1 * oprand2; break; }
	case Action::DIV: {*res = oprand1 / oprand2; break; }
	case Action::REM: {*res = oprand1 % oprand2; break; }
	default:
		break;
	}
	oprandStack.push(res);
	unused.push_back(res);
}

//执行表达式的action并返回产生式头
SymbolWithAttr Parser::ExecuteAction(int pItr) {
	auto& p = (*grammer)[pItr];
	vector<int> postfix;
	vector<int*> unused;
	stack<int> opStack;
	stack<void*>oprandStack;
	SymbolWithAttr headSwa(p[0]);
	int attrIndex;
	for (auto &action : p.actions) {
		//构建后缀表达式
		for (int i = 0; i < action->requested.size(); ++i) {
			auto elem = action->requested[i];
			//处理运算操作
			if (elem < 0) {
				//以下操作保证了运算优先级
				//如果操作栈为空，将当前操作压入栈中
				if (opStack.empty()) opStack.push(elem);
				//否则，将当前操作与操作栈顶比较，如果当前操作优先级高，将当前操作压入栈中，
				//代表先进行当前操作
				//如果当前操作比之前的操作优先级相等或更低，将之前操作放入后缀表达式，
				//代表先进行之前操作
				else {
					while (true) {
						if (opStack.empty()) {
							opStack.push(elem);
							break;
						}
						auto preOp = opStack.top();
						if (Action::ComparePriority(elem, preOp)) {
							opStack.push(elem);
							break;
						}
						else {
							postfix.push_back(preOp);
							opStack.pop();
						}
					}
				}
			}
			//处理符号.属性
			else {
				auto attrIndex = action->requested[++i];
				if (elem == 0) 
					postfix.push_back((int)&headSwa.attr->at(attrIndex));
				else {
					int attrPtr = (int)GetAttrPtr(pItr, elem, attrIndex);
#ifdef DEBUG
					ASSERT(attrPtr != 0,"Parser: attrPtr is null");
#endif // DEBUG
					postfix.push_back(attrPtr);
				}
			}
		}
		while (!opStack.empty()) { postfix.push_back(opStack.top()); opStack.pop(); }

		//执行运算
		for (int i = 0; i < postfix.size(); ++i) {
			switch (postfix[i])
			{
			case Action::FUN: {break; }
			case Action::ADD: { 
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::MINUS: {
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::MULT: {
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::DIV: {
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::REM: {
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::ASSIGN:{
				int* oprand = (int*)oprandStack.top();
				oprandStack.pop();
				int* res = (int*)oprandStack.top();
				*res = *oprand;
				break;
			}
			default: {
				oprandStack.push((void*)postfix[i]);
				break;
			}
			}
		}
		while (!oprandStack.empty())oprandStack.pop();
		postfix.clear();
	}
	return headSwa;
}

void Parser::PopToken(int cnt) {
	for (int i = 0; i < cnt; i++) {
		symbolStack.pop();
	}
}

int Parser::Reduce(int productionIndex) {

#ifdef _PARSER_ACTION_PRINT
	int preStatus = curStatus;
#endif // _PARSER_ACTION_PRINT

	Production &p = (*grammer)[productionIndex];
	auto body = p.GetBody();
	int topTokenKind;
	int stackOffset = 0;
	for (int i = body.size() - 1; i >= 0; i--) {
		topTokenKind = symbolStack.get(stackOffset--).symbol;
		if (topTokenKind == body[i]) {
			statusStack.pop();
		}
		else {
			cerr << "Parser : Reduce Error \n";
			PrintStackTrace();
			abort();
		}
			
	}
	auto headSwa = ExecuteAction(productionIndex);
	PopToken(body.size());
	curStatus = statusStack.top();
	

#ifdef _PARSER_ACTION_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(curInputToken) << "\n";
	cout << "ACTION : reduce\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "Reduce Prouduction :";
	grammer->operator[](-(action + 1)).Print();
	cout << "\n\nReturn Status : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT

	shift(headSwa);
	curInputToken = (*tokenStream)[tokenIndex].kind;
	return p.GetHead();
}

void Parser::shift(SymbolWithAttr& swa) {
#ifdef _PARSER_ACTION_PRINT
	int preStatus = curStatus;
#endif // _PARSER_ACTION_PRINT

	symbolStack.push(swa);
	curStatus = collectionPtr->Goto(curStatus, swa.symbol);
	statusStack.push(curStatus);

#ifdef _PARSER_ACTION_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(swa.symbol) << "\n";
	cout << "ACTION : shift\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "\nGoto : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT
}

void Parser::shift(int symbol){

#ifdef _PARSER_ACTION_PRINT
	int preStatus = curStatus;
#endif // _PARSER_ACTION_PRINT
	if (Grammer::IsTerminal(symbol))
		symbolStack.push(SymbolWithAttr((*tokenStream)[tokenIndex]));
	else if (Grammer::IsUnterminal(symbol))
		symbolStack.push(SymbolWithAttr(symbol));
	curStatus = collectionPtr->Goto(curStatus, symbol);
	statusStack.push(curStatus);

#ifdef _PARSER_ACTION_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(curInputToken) << "\n";
	cout << "ACTION : shift\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "\nGoto : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT
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
		shift(curInputToken);
		curInputToken = (*tokenStream)[tokenIndex].kind;
		return true;
	}
	return false;
}

bool Parser::Analyse() {
	if (tokenStream == nullptr) {
		cerr << "Parser: hasn't Input stream\n";
		abort();
	}
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

#ifdef _PARSER_STACK_PRINT
			while (!statusStack.empty()) {
				auto i = statusStack.top();
				collectionPtr->PrintStatus(i);
				cout << "\n\n";
				statusStack.pop();
			}
#endif // _PARSER_STACK_PRINT

			return false;
		}
		else if (action == Collection::ACCESS) 
			return true;
		//shift操作
		else if (action >= 0) {
			shift(curInputToken);
			++tokenIndex;
			curInputToken = (*tokenStream)[tokenIndex].kind;
		}
		//reduce操作
		else {
			//注意 在PraserTable中-1代表使用Production0进行归约
			Reduce(-(action + 1));
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