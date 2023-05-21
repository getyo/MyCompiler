#pragma once
#include <string>
#include <shlwapi.h>
#include <direct.h>
#include <Windows.h>
using namespace std;

class FileManager
{
private:
	static FileManager * fileMangerPtr;
	string workPath;
	bool _IsDir(string dirPath);
	bool _CreateDir(string relativePath);
	FileManager();
	FileManager(const FileManager&) {}
	void operator=(const FileManager&) {}
	~FileManager(){}
public:
	static FileManager* FileManagerFactory();
	bool ChangeWorkPath(string path);
	string getWorkPath();
	bool IsDir(string dirPath);
	bool CreateDir(string relativePath);
	static void Release() { 
		if (fileMangerPtr != nullptr) {
			delete fileMangerPtr;
			fileMangerPtr = nullptr;
		}
	}
};

class FileNotOpen :exception {
private:
	string error = "File cannot open";
public:
	string what();
};