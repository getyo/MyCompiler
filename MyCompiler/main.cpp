#include "Lex.h"
#include "Token.h"
#include "FileManager.h"
#include "Parser.h"
#include "Triple.h"
#include "Debug.h"
#include "Type.h"
#include "Assembly.h"
#include "ControlBlock.h"

CB cb;
SymbolTable symbolTable;
FileManager* fileManager;
Generator* genPtr;
CodeStore cs;
string sourceFile = "";
string outputFile = "";
AssemblyStore as;
/*
* 关于参数的规定：
* 程序名：mcc
* 基本格式 mcc sourcefile [-para]
* -o 指定输出文件名字以及路径；默认为输入文件.s
* -g 打印语法
* -p 打印分析器信息
* -ic 打印中间代码
*/

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Should input source file \n";
		exit(1);
	}
	sourceFile = argv[1];
	string srcName = sourceFile.substr(0,sourceFile.find_last_of('.'));
	outputFile = srcName + ".s";
	for (int i = 2; i < argc; ++i) {
		if (!strcmp(argv[i], "-g"))
			cb.PrintGrammer = true;
		else if (!strcmp(argv[i], "-p"))
			cb.PrintParser = true;
		else if (!strcmp(argv[i], "-ic"))
			cb.PrintIC = true;
		else if (!strcmp(argv[i], "-o")) {
			++i;
			if (i >= argc) {
				cout << "Should input output file name\n";
				exit(1);
			}
			outputFile = argv[i];
		}
		else {
			cout << "Undefined argument " + string(argv[i]) + "\n";
			exit(1);
		}
	}
	//cb.PrintGrammer = true;
	//cb.PrintIC = true;
	//cb.PrintParser = true;
	fileManager = FileManager::FileManagerFactory();
	Lexeme* lex = Lexeme::LexemeFactory();
	lex->InitLex();
	Parser* parser = Parser::ParserFactory();
	genPtr = Generator::GeneratorFactory(cs);


	ifstream* sourceCode = new ifstream(sourceFile);
	if (!sourceCode->is_open()) {
		cerr << "input file cann't open : " << sourceFile;
	}
		
	lex->SetInput(*sourceCode);
	auto tokenStream = lex->Analyse();
	lex->PrintError();


	parser->SetInput(tokenStream);
	bool syntalPass = parser->Analyse();
	if (!syntalPass) parser->PrintError();


	if (cb.PrintIC) {
		cout << endl;
		Generator::Print();
		cout << endl;
		Environment::PrintAll();
	}
	AssGen* assGen = AssGen::AssGenFactory(as);
	assGen->SetInput(cs);
	assGen->Gen();

	ofstream o(outputFile);
	auto oldbuf = std::cout.rdbuf(o.rdbuf());
	assGen->Print();
	std::cout.rdbuf(oldbuf);
	o.close();

	AssGen::Release();
	Generator::Release();
	Parser::Release();
	Lexeme::Release();
	FileManager::Release();
}