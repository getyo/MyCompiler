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
public:
	static FileManager* FileManagerFactory();
	bool ChangeWorkPath(string path);
	string getWorkPath();
	bool IsDir(string dirPath);
	bool CreateDir(string relativePath);
};

class FileNotOpen :exception {
private:
	string error = "File cannot open";
public:
	string what();
};