#pragma once
#include <string>
#include <shlwapi.h>
#include <direct.h>
#include <Windows.h>
using namespace std;
class FileManager
{
private:
	string workPath;
	bool _IsDir(string dirPath);
	bool _CreateDir(string relativePath);
public:
	FileManager();
	bool ChangeWorkPath(string path);
	string getWorkPath();
	bool IsDir(string dirPath);
	bool CreateDir(string relativePath);
};
