// InstallAFGD.cpp : Defines the entry point for the console application.
//


#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <shlobj.h>
using namespace std;
const char* auto_future_gui_design = "HSG";
const char* auto_future_gui_icon = "af_gd.ico,0";
int main(int argc, char* argv[])
{
	string file_path;
	string icon_path;
	if (argc<2)
	{
		TCHAR currentDirectoty[MAX_PATH];
		DWORD dwRet = GetCurrentDirectory(MAX_PATH, currentDirectoty);
		file_path = currentDirectoty;
		file_path += "\\";
		icon_path = file_path;
		icon_path += auto_future_gui_icon;
		file_path += auto_future_gui_design;
		file_path += ".exe";
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile(file_path.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("missing HSG.exe!\n");
			return 0;
		}
		FindClose(hFind);
	}
	HKEY hKey;
	LPCTSTR lpRun = ".afg";
	DWORD state;
	long lRet;
	char reBuff[10] = { 0 };


	/*������***************************************************************************/
	////������
	//lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT,lpRun,0,NULL,0,0,NULL,&hKey,&state);
	lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, lpRun, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &state);
	if(lRet == ERROR_SUCCESS)
	{
	    if(state == REG_CREATED_NEW_KEY)
	        cout<<"������ɹ�"<<endl;
	
	    //�رռ�
		lRet=RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)auto_future_gui_design, strlen(auto_future_gui_design));
	    RegCloseKey(hKey);
	}
	string shellcmd = auto_future_gui_design;
	shellcmd += "\\shell\\open\\command";
	string shellcmdValue = file_path + " %1";
	lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, shellcmd.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, &state);
	if(lRet == ERROR_SUCCESS)
	{
	    if(state == REG_CREATED_NEW_KEY)
	        cout<<"������ɹ�"<<endl;
	
	    //�رռ�
		lRet = RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)shellcmdValue.c_str(), shellcmdValue.length());
	    RegCloseKey(hKey);
	}
	string strIcon = auto_future_gui_design;
	strIcon += "\\DefaultIcon";
	//strIcon += auto_future_gui_icon;
	lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, strIcon.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, &state);
	if(lRet == ERROR_SUCCESS)
	{
	    if(state == REG_CREATED_NEW_KEY)
	        cout<<"������ɹ�"<<endl;
	
	    //�رռ�
		lRet = RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)icon_path.c_str(), icon_path.length());
	    RegCloseKey(hKey);
	}
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	/*��/�޸ļ�ֵ********************************************************************/
	////�򿪼�
	//lRet= RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey);
	//if(lRet == ERROR_SUCCESS) 
	//{     
	//    //���������޸ļ�ֵ
	//    RegSetValueEx(hKey, "test",0,REG_SZ,(BYTE *)"success",10);
	//    
	//    //�رռ�    
	//    RegCloseKey(hKey);
	//}


	/*��ȡ��ֵ*************************************************************************/
	////�򿪼�
	//lRet= RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_READ, &hKey);
	//if(lRet == ERROR_SUCCESS) 
	//{
	//    sizeBuff = sizeof(reBuff);
	//    
	//    //��ȡ��ֵ
	//    if(RegQueryValueEx(hKey,"test",0,&dwtype,(BYTE*)reBuff,&sizeBuff) == ERROR_SUCCESS)
	//        cout<<reBuff<<endl;
	//    
	//    //�رռ�
	//    RegCloseKey(hKey);
	//}


	/*ɾ����ֵ************************************************************************/
	////�򿪼�
	//lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey);    
	//if(lRet==ERROR_SUCCESS)
	//{
	//    //ɾ����
	//    RegDeleteValue(hKey,"test");
	//    
	//    //�رռ�
	//    RegCloseKey(hKey);
	//}


	/*ɾ����**************************************************************************/

	system("pause");
	return 0;
}

