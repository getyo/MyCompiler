#include "Parser.h"
#include <stack>
#include "Debug.h"
#include "Type.h"
#include "Triple.h"
#include "AuxiliaryFunction.h"
Parser* Parser::parserPtr = nullptr;
extern SymbolTable symbolTable;
int Parser::row;
int Parser::col;
extern Generator* genPtr;

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

//ʹ�øú���ʱĬ�Ϸ���ջ�Լ����иñ��ʽȫ������
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

//ִ�б��ʽ��action�����ز���ʽͷ
SymbolWithAttr Parser::ExecuteAction(int pItr) {
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
	for (auto &action : p.actions) {
		//������׺���ʽ
		for (int i = 0; i < action->requested.size(); ++i) {
			auto elem = action->requested[i];
			//�����������
			if (elem < 0) {
				//���²�����֤���������ȼ�
				//�������ջΪ�գ�����ǰ����ѹ��ջ��
				if (opStack.empty()) {
					opStack.push(elem);
				}
				//���򣬽���ǰ���������ջ���Ƚϣ������ǰ�������ȼ��ߣ�����ǰ����ѹ��ջ�У�
				//�����Ƚ��е�ǰ����
				//�����ǰ������֮ǰ�Ĳ������ȼ���Ȼ���ͣ���֮ǰ���������׺���ʽ��
				//�����Ƚ���֮ǰ����
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
			//�������.����
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

		//ִ������
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
	auto headSwa = ExecuteAction(productionIndex);

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
	cout << "\n\n";
#endif // _PARSER_ACTION_PRINT

	PopToken(body.size());
	curStatus = statusStack.top();
	shift(headSwa);
	curInputToken = (*tokenStream)[tokenIndex].kind;
	return p.GetHead();
}

void Parser::shift(SymbolWithAttr& swa) {
#ifdef _PARSER_SHIFT_PRINT
	int preStatus = curStatus;
#endif // _PARSER_ACTION_PRINT

	symbolStack.push(swa);
	curStatus = collectionPtr->Goto(curStatus, swa.symbol);
	statusStack.push(curStatus);

#ifdef _PARSER_SHIFT_PRINT
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

#ifdef _PARSER_SHIFT_PRINT
	int preStatus = curStatus;
#endif // _PARSER_ACTION_PRINT
	if (Grammer::IsTerminal(symbol))
		symbolStack.push(SymbolWithAttr((*tokenStream)[tokenIndex]));
	else if (Grammer::IsUnterminal(symbol))
		symbolStack.push(SymbolWithAttr(symbol));
	curStatus = collectionPtr->Goto(curStatus, symbol);
	statusStack.push(curStatus);

#ifdef _PARSER_SHIFT_PRINT
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
		//ע�� ��PraserTable��-1����ʹ��Production0���й�Լ
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

	Environmemt::curEnv = Environmemt::NewEnv();
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
		//shift����
		else if (action >= 0) {
			shift(curInputToken);
			int symTableIt = (*tokenStream)[tokenIndex].symbolTableIndex;
			row = symbolTable[symTableIt].row;
			col = symbolTable[symTableIt].col;
			++tokenIndex;
			curInputToken = (*tokenStream)[tokenIndex].kind;
		}
		//reduce����
		else {
			//ע�� ��PraserTable��-1����ʹ��Production0���й�Լ
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
		if (head.attr->at(i) != -1)
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
			auto symStr = Grammer::GetSymbolStr(swa.symbol);
			cout << symStr + ".lexeme : " \
				+ symbolTable[swa.symTableIndex].lexeme + " \t";
			if (symbolTable[swa.symTableIndex].val != -1)
				cout << symStr + ".val: " + to_string(symbolTable[swa.symTableIndex].val) + " \t";
			if (symbolTable[swa.symTableIndex].typeID != -1)
				cout << symStr + ".typeID: " + Type::GetTypeStr(symbolTable[swa.symTableIndex].typeID) + " \t";
		}
		else if (Grammer::IsUnterminal(swa.symbol)) {
			auto symStr = Grammer::GetSymbolStr(swa.symbol);
			for (int i = 0; i < 8; i++) {
				if (swa.attr->at(i) != -1)
					cout << symStr + '.' + Grammer::GetAttrStr(swa.symbol, i) + " : "\
					+ to_string(swa.attr->at(i)) + " \t";
				else break;
			}
			cout << headStr + '.' + Grammer::GetAttrStr(head.symbol, 6) + " : "\
				+ to_string(swa.attr->at(6)) + " \t";
			cout << headStr + '.' + Grammer::GetAttrStr(head.symbol, 7) + " : "\
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