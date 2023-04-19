#pragma once
#include <string>
#include <shlwapi.h>
#include <direct.h>
#include <Windows.h>
using namespace std;
class FileManager
{
public:
	static bool IsDir(string dirPath);
	static bool CreateMultDir(string relativePath);
	static bool CreateDir(string relativePath);
};

