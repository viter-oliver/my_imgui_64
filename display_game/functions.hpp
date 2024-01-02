#include <string>
#include <vector>
#include <io.h>
#include<direct.h>

bool fileExist(const char* fileName)
{
	WIN32_FIND_DATA wfd;
	HANDLE hHandle = ::FindFirstFile(fileName, &wfd);
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;
	else
		return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;

}

bool directoryExist(const char* dir)
{
	WIN32_FIND_DATA wfd;
	HANDLE hHandle = ::FindFirstFile(dir, &wfd);
	if (hHandle == INVALID_HANDLE_VALUE)
		return access(dir, 0) == 0; // if dir is a drive disk path like c:\,we thought is a directory too.  	
	else
		return (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}


bool createDirectory(const char* pathName)
{
	char path[MAX_PATH] = { 0 };
	const char* pos = pathName;
	while ((pos = strchr(pos, '\\')) != NULL)
	{
		memcpy(path, pathName, pos - pathName + 1);
		pos++;
		if (directoryExist(path))
		{
			continue;
		}
		else
		{
			int ret = _mkdir(path);
			if (ret == -1)
			{
				return false;
			}
		}
	}
	pos = pathName + strlen(pathName) - 1;
	if (*pos != '\\')
	{
		return _mkdir(pathName) == 0;
	}
	return true;
}
bool createFileWithDirectory(const char* pathName)
{
	if (fileExist(pathName))
		return true;
	int len = strlen(pathName);
	if (len <= 0)
		return false;

	char strTmpPath[MAX_PATH] = { 0 };
	strcpy(strTmpPath, pathName);
	char* q = strTmpPath + len - 1;
	for (int i = 0; i < len - 1; i++, q--)
	{
		if (*q == '\\')
		{
			*q = '\0';
			q++;
			break;
		}
	}
	if (strlen(strTmpPath) > 0 && strlen(q) > 0)
	{
		createDirectory(strTmpPath);
		FILE* hFile = fopen(pathName, "w");
		if (hFile)
		{
			fclose(hFile);
			return true;
		}
		else
			return false;

	}
	else
	{
		return false;
	}
}
void spilt_str(std::string& content, std::vector<std::string>& sub_content_list, char by_ch) {
	std::string s;
	for (auto& x : content) {
		if (x == by_ch) {
			sub_content_list.push_back(string());
			auto& tail_str = *(sub_content_list.end() - 1);
			tail_str = s;
			s = "";
		}
		else {
			s += x;
		}
	}
}
