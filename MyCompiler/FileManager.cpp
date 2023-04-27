#include "FileManager.h"
FileManager* FileManager::fileMangerPtr = nullptr;

FileManager* FileManager::FileManagerFactory() {
	if (fileMangerPtr != nullptr)
		return fileMangerPtr;
	else {
		fileMangerPtr = new FileManager();
		return fileMangerPtr;
	}
}

FileManager::FileManager() {
	char buf[265];
	_getcwd(buf, 256);
	workPath = string(buf);
}

string FileManager::getWorkPath() {
	return workPath;
}

bool FileManager::IsDir(string dirPath) {
	return _IsDir(getWorkPath() + "\\" + dirPath);
}

bool FileManager::_IsDir(string dirPath) {
	DWORD ftyp = GetFileAttributesA(dirPath.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path! 
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory! 
	return false;    // this is not a directory! 
}

bool FileManager::ChangeWorkPath(string path) {
	int flag = _chdir(path.c_str());
	if (flag == -1) return false;
	return true;
}

bool FileManager::CreateDir(string relativePath) {
	int sub = relativePath.find_first_of('\\', 1);
	if (sub == -1) {
		return _CreateDir(relativePath);
	}

	string curPath = getWorkPath();
	bool createCur = _CreateDir(relativePath.substr(0, sub));

	string  nextPath = getWorkPath();
	nextPath +=("\\" + relativePath.substr(0, sub));

	ChangeWorkPath(nextPath);
	bool createNext = CreateDir(relativePath.substr(sub+1, relativePath.length()));
	ChangeWorkPath(curPath);

	return createCur && createNext;

}

bool FileManager::_CreateDir(string relativePath) {
	char buf[256];
	_getcwd(buf, 256);
	strcat_s(buf, "\\");				//在当前路径后面加"\"
	strcat_s(buf, relativePath.c_str());				//在"\"后面加文件夹名
	if (IsDir(buf)) return true;
	return CreateDirectoryA(buf, NULL);
}

string FileNotOpen::what() {
	return error;
}