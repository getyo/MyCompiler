#pragma once
#include <string>
#include <shlwapi.h>
#include <direct.h>
#include <Windows.h>
using namespace std;
class FileManager
{
public:
	static bool IsDir(string dirPath) {
		DWORD ftyp = GetFileAttributesA(dirPath.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path! 
		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory! 
		return false;    // this is not a directory! 
	}
	static bool CreateMultDir(string relativePath) {
		if (relativePath[0] == '\0') return true;
		int sub = relativePath.find_first_of('\\', 1);
		return CreateDir(relativePath.substr(0, sub)) && 
			CreateMultDir(relativePath.substr(sub,relativePath.length()-1));

	}
	static bool CreateDir(string relativePath) {
		char buf[256];
		_getcwd(buf, 256);
		strcat_s(buf, "\\");				//在当前路径后面加"\"
		strcat_s(buf, relativePath.c_str());				//在"\"后面加文件夹名
		if (IsDir(buf)) return true;
		return CreateDirectoryA(buf, NULL);
	}
};

