#include "Production.h"
#include "Debug.h"
#include <string>
#include "Grammer.h"


Production::Production(const Production && p) {
	this->head = p.head;
	this->body = p.body;
}

Production::Production(const Production& p) {
	head = p.head;
	body = p.body;
}

Production::Production(vector<int>& p) {
	this->head = p[0];
	this->body.insert(body.begin(), p.begin() + 1, p.end());
}

Production::Production(initializer_list<int> l) {
	this->head = *l.begin();
	for (auto it = l.begin() + 1; it != l.end(); it++)
		body.push_back(*it);
}

Production::Production(int head, vector<int>& body) {
	this->head = head;
	this->body = body;
}

int Production::GetHead() const{
	return this->head;
}

vector<int> Production::GetBody() const{
	return this->body;
}

int& Production::operator[](size_t index) {
	if (!index) return head;
	else return body[index - 1];
}

string Production::Info() {
	string s = "";
	s += (to_string(head) + " ");
	for (auto &i : body) {
		s += (to_string(i) + " ");
	}
	return s;
}

void Production::Print() {
	cout << Grammer::grammerSymbolNum2Str[head] \
		<< ' ' << "-> ";
	for (auto& i : body)
		cout << Grammer::grammerSymbolNum2Str[i] << " ";
}

Item::Item(Production&& p) :Production(p) {
	//默认点在产生式体开头，利用prodction的[]可以直接返回dot后的语法符号
	this->dotPos = 1;
}

Item::Item(Production&& p, initializer_list<char> l) : Production(p) {
	this->dotPos = 1;
	for (auto c : l) lookAhead.insert(c);
}

void Item::AddLookAhead(char c) {
	lookAhead.insert(c);
}

bool Item::FindLookAhead(char c) const {
	return lookAhead.count(c);
}

void Item::AddDotPos() {
	++dotPos;
}