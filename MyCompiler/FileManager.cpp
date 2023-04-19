#include "FileManager.h"

bool FileManager::IsDir(string dirPath) {
	DWORD ftyp = GetFileAttributesA(dirPath.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path! 
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory! 
	return false;    // this is not a directory! 
}

bool CreateMultDir(string relativePath) {
	if (relativePath[0] == '\0') return true;
	int sub = relativePath.find_first_of('\\', 1);
	return CreateDir(relativePath.substr(0, sub)) &&
		CreateMultDir(relativePath.substr(sub, relativePath.length() - 1));

}

bool CreateDir(string relativePath) {
	char buf[256];
	_getcwd(buf, 256);
	strcat_s(buf, "\\");				//�ڵ�ǰ·�������"\"
	strcat_s(buf, relativePath.c_str());				//��"\"������ļ�����
	if (IsDir(buf)) return true;
	return CreateDirectoryA(buf, NULL);
}