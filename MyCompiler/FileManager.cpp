#include "FileManager.h"

bool FileManager::IsDir(string dirPath) {
	DWORD ftyp = GetFileAttributesA(dirPath.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path! 
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory! 
	return false;    // this is not a directory! 
}

bool FileManager::CreateMultDir(string relativePath) {
	int sub = relativePath.find_first_of('\\', 1);
	if (sub == -1) {
		return CreateDir(relativePath);
	}

	char nextBuf[256],curBuf[256];
	bool createCur = CreateDir(relativePath.substr(0, sub));
	string nextWorkDir = "\\";
	nextWorkDir += relativePath.substr(0, sub);
	_getcwd(nextBuf, 256);
	_getcwd(curBuf, 256);
	strcat_s(nextBuf, nextWorkDir.c_str());
	_chdir(nextBuf);
	bool createNext = CreateMultDir(relativePath.substr(sub+1, relativePath.length() - 1));
	_chdir(curBuf);
	return createCur && createNext;

}

bool FileManager::CreateDir(string relativePath) {
	char buf[256];
	_getcwd(buf, 256);
	strcat_s(buf, "\\");				//在当前路径后面加"\"
	strcat_s(buf, relativePath.c_str());				//在"\"后面加文件夹名
	if (IsDir(buf)) return true;
	return CreateDirectoryA(buf, NULL);
}