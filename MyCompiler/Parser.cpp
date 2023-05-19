#include "Parser.h"
#include <stack>
#include "Debug.h"
#include "Type.h"
#include "Triple.h"
#include "AuxiliaryFunction.h"
#include <string>
Parser* Parser::parserPtr = nullptr;
extern SymbolTable symbolTable;
int Parser::row;
int Parser::col;
extern Generator* genPtr;


void SymbolStack::Print() {
	for (int i = 0; i <= topIndex; ++i) {
		if (array[i].symbol == PH_SYM) cout << "PlaceHolder" << " ";
		else cout << Grammer::GetSymbolStr(array[i].symbol) << " ";
	}
	cout << endl;
}

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

void* Parser::GetAttrPtr(int pItr, int smbIndex, int attrIndex) {
	auto& p = (*grammer)[pItr];
	int bodysize = p.GetBody().size();
	int symID;
	bool isInhAction = true;
	if (bodysize == dotPosStack.top()) isInhAction = false;

	if (smbIndex >= 0) symID = p[smbIndex];
	else {
#ifdef DEBUG
		ASSERT(smbIndex == -1, "Parser: illegal symbol Index");
#endif // DEBUG
		int offset = - dotPosStack.top();
		//如果是继承属性使用继承属性的情况下，由于当前栈中除了dotPos个符号外还压入了一个
		//PalceHolder，所以需要将偏移量+1才能获取到表达式头的继承属性
		if (symbolStack.top().symbol == -1) --offset;
		auto& swa = symbolStack.get(offset);
		return &swa.attr->at(attrIndex);
	}
	int offset;
	if (!isInhAction) offset = smbIndex - bodysize;
	else {
		offset = smbIndex - dotPosStack.top();
		if (symbolStack.top().symbol == -1) --offset;
	}
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
	int oprand1,oprand2;
	if ((int)oprandStack.top() == Action::DIGIT) {
		oprandStack.pop();
		oprand1 = (int)oprandStack.top();
	}
	else oprand1 = *(int*)oprandStack.top();
	oprandStack.pop();

	if ((int)oprandStack.top() == Action::DIGIT) {
		oprandStack.pop();
		oprand2 = (int)oprandStack.top();
	}
	else oprand2 = *(int*)oprandStack.top();
	oprandStack.pop();

	int* res = new int();
	switch (op)
	{
	case Action::ADD: {
		*res = oprand1 + oprand2; break; }
	case Action::MINUS: {*res = oprand1 - oprand2; break; }
	case Action::MULT: {
		*res = oprand1 * oprand2; break; }
	case Action::DIV: {*res = oprand1 / oprand2; break; }
	case Action::REM: {*res = oprand1 % oprand2; break; }
	default:
		break;
	}
	oprandStack.push(res);
	unused.push_back(res);
}

//执行表达式的action并返回产生式头
SymbolWithAttr Parser::ExecuteAction(int pItr,int dotPos) {
	auto& p = (*grammer)[pItr];
	vector<int> postfix;
	vector<int*> unused;
	stack<int> opStack;
	stack<void*>oprandStack;
	SymbolWithAttr headSwa(p[0]);
	int attrIndex;
	bool isFunPara = false;
	int curParaIndex = 0;
	int curfunID;
	for (auto &action : p.actionLists[dotPos]) {
		//构建后缀表达式
		for (int i = 0; i < action->requested.size(); ++i) {
			auto elem = action->requested[i];
			//处理运算操作
			if (elem < 0 && elem != -1) {
				//以下操作保证了运算优先级
				//如果操作栈为空，将当前操作压入栈中
				if (opStack.empty()) {
					opStack.push(elem);
				}
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
				if (elem == Action::DIGIT) {
					postfix.push_back(action->requested[++i]);
#ifdef DEBUG
					ASSERT(opStack.top() == Action::DIGIT, "Parser : opStack Error");
#endif // DEBUG
					opStack.pop();
					postfix.push_back(Action::DIGIT);
					if (isFunPara) {
						++curParaIndex;
						if (curParaIndex >= Grammer::GetFunParaCnt(curfunID)) {
							postfix.push_back(curfunID);
							isFunPara = false;
						}
					}
				}
				else if (elem == Action::OP) {
					postfix.push_back(action->requested[++i]);
#ifdef DEBUG
					ASSERT(opStack.top() == Action::OP, "Parser : opStack Error");
#endif // DEBUG
					opStack.pop();
					postfix.push_back(Action::OP);
					if (isFunPara) {
						++curParaIndex;
						if (curParaIndex >= Grammer::GetFunParaCnt(curfunID)) {
							postfix.push_back(curfunID);
							isFunPara = false;
						}
					}
				}
				else if (elem == Action::FUN) {
					isFunPara = true;
					curParaIndex = 0;
					curfunID = action->requested[++i];
					if (!Grammer::GetFunParaCnt(curfunID)) {
						postfix.push_back(curfunID);
						isFunPara = false;
					}
				}
			}
			//处理符号.属性
			else {
				auto attrIndex = action->requested[++i];
				if (elem == 0) 
					postfix.push_back((int)&headSwa.attr->at(attrIndex));
				else {
					if (elem == 1 && attrIndex == 0 && pItr == 60)
						cout << "";
					int attrPtr = (int)GetAttrPtr(pItr, elem, attrIndex);
#ifdef DEBUG
					ASSERT(attrPtr != 0,"Parser: attrPtr is null");
#endif // DEBUG
					postfix.push_back(attrPtr);
				}
				if (isFunPara) {
					++curParaIndex;
					if (curParaIndex >= Grammer::GetFunParaCnt(curfunID)) {
						postfix.push_back(curfunID);
						isFunPara = false;
					}
				}
			}
		}
		while (!opStack.empty()) { postfix.push_back(opStack.top()); opStack.pop(); }

		//执行运算
		for (int i = 0; i < postfix.size(); ++i) {
			switch (postfix[i])
			{
			case Action::DIGIT: {
				oprandStack.push((void*)Action::DIGIT);
				break;
			}
			case Action::OP: {
				oprandStack.push((void*)Action::OP);
				break;
			}
			case Action::FUN: {
				int funID = (int)oprandStack.top();
				oprandStack.pop();
				int paraCnt = Grammer::GetFunParaCnt(funID);
				void* paraList[MAX_PARACNT];
				for (int paraIndex = paraCnt-1; paraIndex >=0 ; --paraIndex) {
					if ((int)oprandStack.top() == Action::DIGIT || (int)oprandStack.top() == Action::OP)
						oprandStack.pop();
					paraList[paraIndex] = oprandStack.top();
					oprandStack.pop();
				}
				int res;
				FUNCALL(funID, paraList, res);
				oprandStack.push((void*)res);
				oprandStack.push((void*)Action::DIGIT);
				break; 
			}
			case Action::ADD: { 
				ComputedOp(oprandStack, unused, Action::ADD);
				break; }
			case Action::MINUS: {
				ComputedOp(oprandStack, unused, Action::MINUS);
				break; }
			case Action::MULT: {
				ComputedOp(oprandStack, unused, Action::MULT);
				break; }
			case Action::DIV: {
				ComputedOp(oprandStack, unused, Action::DIV);
				break; }
			case Action::REM: {
				ComputedOp(oprandStack, unused, Action::REM);
				break; }
			case Action::ASSIGN:{
				int oprand;
				if ((int)oprandStack.top() == Action::DIGIT || (int) oprandStack.top() == Action::OP) {
					oprandStack.pop();
					oprand = (int)oprandStack.top();
					oprandStack.pop();
					int* res = (int*)oprandStack.top();
					oprandStack.pop();
					*res = oprand;
					oprandStack.push(res);
				}
				else {
					int* oprand = (int*)oprandStack.top();
					oprandStack.pop();
					int* res = (int*)oprandStack.top();
					oprandStack.pop();
					*res = *oprand;
					oprandStack.push(res);
				}
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
	for (auto& i : unused) delete i;
	return headSwa;
}

void Parser::PopToken(int cnt) {
	for (int i = 0; i < cnt; i++) {
		symbolStack.pop();
	}
}

int Parser::Reduce(int productionIndex) {

#ifdef _PARSER_REDUCE_PRINT
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
	auto headSwa = ExecuteAction(productionIndex,p.GetBody().size());
	curStatus = statusStack.top();

#ifdef _PARSER_REDUCE_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(curInputToken) << "\n";
	cout << "ACTION : reduce\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "Reduce Prouduction :";
	grammer->operator[](-(action + 1)).Print();
	cout << "\n";
	PrintAttr(productionIndex, headSwa);
	cout << "\n\nReturn Status : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
#endif

	PopToken(body.size() + 1);
	dotPosStack.pop();

#ifdef _PARSER_REDUCE_PRINT
	cout << "\nSymbolStack : \t";
	symbolStack.Print();
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT

	shift(headSwa);
	curInputToken = (*tokenStream)[tokenIndex].kind;
	return p.GetHead();
}


void Parser::shift(SymbolWithAttr& swa) {

	int preStatus = curStatus;
	//跳转状态
	curStatus = collectionPtr->Goto(curStatus, swa.symbol);
	statusStack.push(curStatus);

	//移入符号
	if (NewProduction(swa.symbol,curStatus,preStatus)) {
		//如是新的表达式，需要压入PlaceHolder，用于继承属性的占位
		symbolStack.push(SymbolWithAttr(PH_SYM));
		//执行继承Acion
		int curDotPos = dotPosStack.top();
		auto itemSet = collectionPtr->GetStatus(preStatus);
		for (auto& item : itemSet) {
			for (auto& pair : item.actionLists) {
				if (pair.first == curDotPos)
					ExecuteAction(item.GetPItr() - 1, curDotPos); 
			}
		}
		//重新开始dotPos的计数
		dotPosStack.push(0);
	}

	symbolStack.push(swa);
	dotPosStack.top() += 1;



#ifdef _PARSER_SHIFT_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(swa.symbol) << "\n";
	cout << "ACTION : shift\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "\nGoto : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
	cout << "\nSymbolStack : \t";
	symbolStack.Print();
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT
}

bool Parser::NewProduction(int inputSymbol,int curStatus,int preStatus) {

	bool hasFollowDot = collectionPtr->HasFollowDot(preStatus, inputSymbol);
	bool hasDotPos1 = collectionPtr->HasDotPos1(curStatus);
#ifdef DEBUG
	ASSERT(hasFollowDot || hasDotPos1, "LOGIC ERROR");
#endif // DEBUG

	if (hasFollowDot && !hasDotPos1) return false;
	else if (!hasFollowDot && hasDotPos1) return true;
	//左递归
	else if (hasDotPos1 && hasFollowDot) {
		int nextToken = (*tokenStream)[tokenIndex].kind;
		int action = collectionPtr->Goto(curStatus,curInputToken);
		if (action < 0) return false;
		else return NewProduction(nextToken, action, curStatus);
	}
}

void Parser::shift(int symbol){

	int preStatus = curStatus;

	curStatus = collectionPtr->Goto(curStatus, symbol);
	statusStack.push(curStatus);

	//移入符号
	if (NewProduction(symbol,curStatus,preStatus)) {
		//如是新的表达式，需要压入PlaceHolder，用于继承属性的占位
		symbolStack.push(SymbolWithAttr(PH_SYM));
		//执行继承Acion
		int curDotPos = dotPosStack.top();
		auto itemSet = collectionPtr->GetStatus(preStatus);
		for (auto& item : itemSet) {
			int pItr = item.GetPItr() - 1;
			auto& p = (*grammer)[pItr];
			for (auto& pair : p.actionLists) {
				if (pair.first == curDotPos && curDotPos < p.GetBody().size())
					ExecuteAction(pItr, curDotPos);
			}
		}
		//重新开始dotPos的计数
		dotPosStack.push(0);
	}

	if (Grammer::IsTerminal(symbol)) {
		symbolStack.push(SymbolWithAttr((*tokenStream)[tokenIndex]));
		//对于NON来说，它并不存在于符号表中，上一步使用NON之后的终结符创建
		//swa，这一步将符号表项置为0
		if (symbol == nonInt) {
			symbolStack.top().symbol = nonInt;
			symbolStack.top().symTableIndex = -1;
		}
	}
	else if (Grammer::IsUnterminal(symbol))
		symbolStack.push(SymbolWithAttr(symbol));
	dotPosStack.top() += 1;

#ifdef _PARSER_SHIFT_PRINT
	cout << "\nInput Token : " << Grammer::GetSymbolStr(curInputToken) << "\n";
	cout << "ACTION : shift\n";
	cout << "\nCurStatus Status : Status " << preStatus << "\n";
	collectionPtr->PrintStatus(preStatus);
	cout << "\n";
	cout << "\nGoto : Status " << curStatus << "\n";
	collectionPtr->PrintStatus(curStatus);
	cout << "\nSymbolStack : \t";
	symbolStack.Print();
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
		Reduce(-(action + 1));
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
	symbolStack.push(SymbolWithAttr(PH_SYM));
	statusStack.push(curStatus);
	curInputToken = (*tokenStream)[tokenIndex].kind;
	action = collectionPtr->Goto(curStatus, curInputToken);;
	dotPosStack.push(0);


	Environment::curEnv = Environment::NewEnv();
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
			int symTableIt = (*tokenStream)[tokenIndex].symbolTableIndex;
			row = symbolTable[symTableIt].row;
			col = symbolTable[symTableIt].col;
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

void Parser::PrintAttr(int pItr, SymbolWithAttr& head) {
	int bodySize = grammer->operator[](pItr).GetBody().size();
	string headStr = Grammer::GetSymbolStr(head.symbol);
	cout << "HeadAttr:\n";
	int attrVal;
	for (int i = 0; i < 8; i++) {
		if (head.attr->at(i) != ATTR_NON)
			cout << headStr + '.' + Grammer::GetAttrStr(head.symbol, i) + " : "\
			+ to_string(head.attr->at(i)) + " \t";
		else break;
	}
	cout << headStr + '.' + Grammer::GetAttrStr(head.symbol,6) + " : "\
		+ to_string(head.attr->at(6)) + " \t";
	cout << headStr + '.' + Grammer::GetAttrStr(head.symbol,7) + " : "\
		+ to_string(head.attr->at(7)) + " \t";
	cout << "\nBodyAttr:\n";

	for (int i = bodySize-1; i >= 0; --i) {
		auto& swa = symbolStack.get(-i);
		if (Grammer::IsTerminal(swa.symbol)) {
			if (swa.symbol == nonInt) {
				cout << "_NON";
				continue;
			}
			auto symStr = Grammer::GetSymbolStr(swa.symbol);
			cout << symStr + ".lexeme : " \
				+ symbolTable[swa.symTableIndex].lexeme + " \t";
			if (symbolTable[swa.symTableIndex].val != ATTR_NON)
				cout << symStr + ".val: " + to_string(symbolTable[swa.symTableIndex].val) + " \t";
			if (symbolTable[swa.symTableIndex].typeID != ATTR_NON)
				cout << symStr + ".typeID: " + Type::GetTypeStr(symbolTable[swa.symTableIndex].typeID) + " \t";
		}
		else if (Grammer::IsUnterminal(swa.symbol)) {
			auto symStr = Grammer::GetSymbolStr(swa.symbol);
			for (int i = 0; i < 8; i++) {
				if (swa.attr->at(i) != ATTR_NON)
					cout << symStr + '.' + Grammer::GetAttrStr(swa.symbol, i) + " : "\
					+ to_string(swa.attr->at(i)) + " \t";
				else break;
			}
			cout << symStr + '.' + Grammer::GetAttrStr(swa.symbol, 6) + " : "\
				+ to_string(swa.attr->at(6)) + " \t";
			cout << symStr + '.' + Grammer::GetAttrStr(swa.symbol, 7) + " : "\
				+ to_string(swa.attr->at(7)) + " \t";
		}

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

string Parser::RowAndCol() {
	return "row : " + to_string(row) + " column : " + to_string(col);
}