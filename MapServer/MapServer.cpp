// MapServer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Map.h"
#include <IOCPCheckClient.h>
#include <atlstr.h>
#include <tlhelp32.h>
#include "Injector.h"
#ifdef ENCRYPT_VMP
#include "VMProtectSDK.h"
#endif
extern Map GameMap;
extern TownMap townmap;
extern SimpleMap PollutantMap;
extern HWND hGameWnd;
//连接
//更新地图
#define _UpdatePollutantMap CrakerDajijiji
extern "C"__declspec(dllexport) int WINAPI _UpdatePollutantMap()
{
	return PollutantMap.ResetMapInfo();
}
#define _UpdateMap CrakerDajiji
extern "C"__declspec(dllexport) int WINAPI _UpdateMap(bool GroupModel)
{
	return GameMap.ResetMapInfo(GroupModel);
}
#define _HitKey helloCraker
extern "C" __declspec(dllexport) void WINAPI _HitKey(int nKey)
{
	MessageBox(NULL, L"222", NULL, MB_OK);
	//HWND hGameWnd = ::FindWindow(L"Direct3DWindowClass", NULL);
	if (hGameWnd == NULL)
	{
		hGameWnd = ::FindWindow(L"Direct3DWindowClass", NULL);
		if (hGameWnd == NULL)
		{
			MessageBox(NULL, L"窗口未初始化,无法发送按键消息", NULL, MB_OK);
			return;
		}
	}
	MessageBox(NULL, L"1111", NULL, MB_OK);
	switch (nKey)
	{
	case 1:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x31, 0x20001);
		PostMessage(hGameWnd, WM_CHAR, 0x31, 0x20001);
		PostMessage(hGameWnd, WM_KEYUP, 0x31, 0xC0020001);
		break;
	case 2:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x32, 0x30001);
		PostMessage(hGameWnd, WM_CHAR, 0x32, 0x30001);
		PostMessage(hGameWnd, WM_KEYUP, 0x32, 0xC0030001);
		break;
	case 3:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x33, 0x40001);
		PostMessage(hGameWnd, WM_CHAR, 0x33, 0x40001);
		PostMessage(hGameWnd, WM_KEYUP, 0x33, 0xC0040001);
		break;
	case 4:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x34, 0x50001);
		PostMessage(hGameWnd, WM_CHAR, 0x34, 0x50001);
		PostMessage(hGameWnd, WM_KEYUP, 0x34, 0xC0050001);
		break;
	case 5:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x35, 0x60001);
		PostMessage(hGameWnd, WM_CHAR, 0x35, 0x60001);
		PostMessage(hGameWnd, WM_KEYUP, 0x35, 0xC0060001);
		break;

	case 6:
		PostMessage(hGameWnd, WM_KEYDOWN, 0xD, 0x1C0001);
		PostMessage(hGameWnd, WM_CHAR, 0xD, 0x1C0001);
		PostMessage(hGameWnd, WM_KEYUP, 0x1C, 0xC01C0001);
		break;
	case 7:
		PostMessage(hGameWnd, WM_KEYDOWN, 0x1B, 0x10001);
		PostMessage(hGameWnd, WM_CHAR, 0x1B, 0x10001);
		PostMessage(hGameWnd, WM_KEYUP, 0x1B, 0xC0010001);
		break;
	}
}
//获取下一探索点(或路径)
#define GetPollutantExplorePoint GetCrakerjjj
extern "C"__declspec(dllexport) int WINAPI GetPollutantExplorePoint(unsigned short& x, unsigned short& y)
{
	Point CurPos;
	CurPos.x = x;
	CurPos.y = y;
	//CString strInfo;
	//strInfo.Format(L"%d,%d", x, y);
	//MessageBox(NULL,strInfo,NULL,MB_OK);
	int nRet = PollutantMap.GetNextExplorePoint(CurPos);
	x = CurPos.x;
	y = CurPos.y;

	return nRet;
}
#define GetExplorePoint GetCrakerjj
extern "C"__declspec(dllexport) int WINAPI GetExplorePoint(unsigned short& x, unsigned short& y)
{
	Point CurPos;
	CurPos.x = x;
	CurPos.y = y;
	//CString strInfo;
	//strInfo.Format(L"%d,%d", x, y);
	//MessageBox(NULL,strInfo,NULL,MB_OK);
	int nRet = GameMap.GetNextExplorePoint(CurPos);
	x = CurPos.x;
	y = CurPos.y;

	return nRet;
}
#define GetGroupExplorePoint CrakerMujj
extern "C"__declspec(dllexport) int WINAPI GetGroupExplorePoint(unsigned short& x, unsigned short& y,int nGroup)
{
	Point CurPos;
	CurPos.x = x;
	CurPos.y = y;
	int nRet = GameMap.GetNextGroupExplorePoint(CurPos, nGroup);
	x = CurPos.x;
	y = CurPos.y;
	return nRet;
}
#define _GetGroupCount GetCrakerJJCount
extern "C"__declspec(dllexport) int WINAPI _GetGroupCount()
{
	//获取
	return GameMap.GetGroupCount();
}
#define _GetCurGroup GetCrakerJJ
extern "C"__declspec(dllexport) int WINAPI _GetCurGroup(unsigned short x, unsigned short y)
{
	return GameMap.GetGroup(x, y);
}
#define _SetGroupExploredPoint SetCrakerBigJJ
extern "C" __declspec(dllexport) void WINAPI _SetGroupExploredPoint(unsigned short x, unsigned short y, int nGroup)
{
	Point TargetPos;
	TargetPos.x = x;
	TargetPos.y = y;
	GameMap.SetGroupExploredPoint(TargetPos, nGroup);
}
//
#define _SetPollutantExploredPoint SetCrakerDajijiji
extern "C" __declspec(dllexport) void WINAPI _SetPollutantExploredPoint(unsigned short x, unsigned short y)
{
	Point TargetPos;
	TargetPos.x = x;
	TargetPos.y = y;
	PollutantMap.SetExploredPoint(TargetPos);
}
#define _SetExploredPoint SetCrakerDajiji
extern "C" __declspec(dllexport) void WINAPI _SetExploredPoint(unsigned short x, unsigned short y)
{
	Point TargetPos;
	TargetPos.x = x;
	TargetPos.y = y;
	GameMap.SetExploredPoint(TargetPos);
}

#define _PollutantMoveToPoint CutCrakerDajijiji
extern "C" __declspec(dllexport) int WINAPI _PollutantMoveToPoint(unsigned short x, unsigned short y, unsigned short px, unsigned short py)
{
	return  PollutantMap.MoveToPoint(x, y, px, py);
}

#define _MoveToPoint CutCrakerDajiji
extern "C" __declspec(dllexport) int WINAPI _MoveToPoint(unsigned short x, unsigned short y, unsigned short px, unsigned short py)
{
	return  GameMap.MoveToPoint(x, y, px, py);
}

#define _SetPollutantPassAbleArea jjjjjjjj
extern "C" __declspec(dllexport) int WINAPI _SetPollutantPassAbleArea(unsigned short x, unsigned short y)
{
	PollutantMap.SetPassAbleArea(x, y);
	return 0;
}

#define _SetPassAbleArea jjjjjjj
extern "C" __declspec(dllexport) int WINAPI _SetPassAbleArea(unsigned short x, unsigned short y)
{
	  GameMap.SetPassAbleArea(x, y);
	  return 0;
}
//画出当前地图
//extern "C"  __declspec(dllexport) void WINAPI DrawMap()
//{
//	townmap.DrawMap();
//}
//
//extern "C"  __declspec(dllexport) void WINAPI GetMapData()
//{
//	GameMap.GetMapData();
//}
#define _GetPollutantAstarDis AAAAAA
extern "C" __declspec(dllexport) int WINAPI _GetPollutantAstarDis(unsigned short x, unsigned short y, unsigned short tx, unsigned short ty)
{
	Point CurPos;
	CurPos.x = x;
	CurPos.y = y;
	Point TargetPos;
	TargetPos.x = tx;
	TargetPos.y = ty;
	return PollutantMap.GetAstarDis(CurPos, TargetPos);
}
#define _GetAstarDis AAAAAAA
extern "C" __declspec(dllexport) int WINAPI _GetAstarDis(unsigned short x, unsigned short y, unsigned short tx, unsigned short ty)
{
	Point CurPos;
	CurPos.x = x;
	CurPos.y = y;
	Point TargetPos;
	TargetPos.x = tx;
	TargetPos.y = ty;
	return GameMap.GetAstarDis(CurPos, TargetPos);
}
//extern "C" __declspec(dllexport) int WINAPI FindPath(unsigned short x, unsigned short y, unsigned short tx, unsigned short ty)
//{
//	map.FindPath(x, y, tx, ty);
//	return 0;
//}
extern "C"__declspec(dllexport) int WINAPI UpdateTownMap(WORD NPC_X, WORD NPC_Y, WORD WP_X, WORD WP_Y, WORD Trans_X, WORD Trancs_Y, WORD StorageX, WORD StorageY)
{
#ifdef ENCRYPT_VMP
	VMProtectBegin("ResetTownMap");
#endif
	return townmap.ResetMapInfo(NPC_X, NPC_Y, WP_X, WP_Y, Trans_X, Trancs_Y, StorageX, StorageY);
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif
}
extern "C"__declspec(dllexport) int WINAPI MoveToSellNPC(WORD x, WORD y)
{
	return townmap.MoveToSellNPC(x, y);
}
extern "C"__declspec(dllexport) int WINAPI  MoveToTownWaypoint(WORD x, WORD y)
{
	return townmap.MoveToTownWaypoint(x, y);
}
extern "C"__declspec(dllexport) int WINAPI  MoveToTransferDoor(WORD x, WORD y)
{
	return townmap.MoveToTransferDoor(x, y);
}
extern "C"__declspec(dllexport) int WINAPI  MoveToStorage(WORD x, WORD y)
{
	return townmap.MoveToStorage(x, y);
}
typedef  ULONG(__stdcall*FP_ReadGarenaAccountBase)(ULONG PID, ULONG);
FP_ReadGarenaAccountBase ReadGarenaAccountBase;
BOOL EnablePrivilege(LPCWSTR lpName, BOOL fEnable);
int WINAPI  MyGetGGPid(char* UID)
{
	EnablePrivilege(SE_DEBUG_NAME, TRUE);
	DWORD dwPid = 0;
	GetUninjectPid(_T("GarenaMessenger.exe"), &dwPid);
	if (dwPid == 0)
		return 1;
	HMODULE hDll = LoadLibraryW(_T("mem.dll"));
	if (hDll == NULL)
		return 2;
	ReadGarenaAccountBase = (FP_ReadGarenaAccountBase)GetProcAddress(hDll, "MapFile");
	DWORD dwAddr = ReadGarenaAccountBase(dwPid, 11);
	DWORD read;
	dwAddr += 4;
	//char buff[32] = { 0 };
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwPid);
	DWORD dwErr = GetLastError();

	if (hProcess == NULL)
		return 3;
	BOOL bRet = ReadProcessMemory(hProcess, (LPCVOID)dwAddr, UID, 32, &read);
	return 0;
}
unsigned int WINAPI CheckThread(void* lpParam)
{
#ifdef ENCRYPT_VMP
	VMProtectBegin("CheckThread");
#endif
#ifdef NDEBUG
	IOCPCheckClient client;
	//验证
	int uid = 0;
	int sessionKey = 0;
//	MessageBoxA(NULL, townmap.szUID, townmap.szPWD,MB_OK);
	int nRet = client.LoginA(townmap.szUID, townmap.szPWD, &uid, &sessionKey);
	if (nRet != 0)
	{
		//	MessageBox(NULL,L"登录失败",L"",MB_OK);
		//	::ExitProcess(0);
		//程序被破解,直接退出
		return 0;
	}
	int nCheckGmaeIDRet = 0;
	while (true)
	{
	/*	char szUID[64] = { 0 };
		MyGetGGPid(szUID);
		if (memcmp(szUID, townmap.szUID, 42) != 0)
			break;*/

		nRet = client.ClientUpdate(uid, &sessionKey);
		if (nRet != 0)
		{
			if (nRet == ANOTHER_USER_LOGIN)
			{
				MessageBox(NULL, L"账户已经在其他地方登录,请重新登录", L"错误", MB_OK);
			}
			else if (INVALID_TIME == nRet)
			{
				MessageBox(NULL, L"时间已到期,请充值", L"错误", MB_OK);
			}
			else if (nRet == CONNECT_SERVER_FAIL)
			{
				MessageBox(NULL, L"无法连接验证服务器", L"错误", MB_OK);
			}
			::ExitProcess(0);
			break;
		}
		//	nCheckGmaeIDRet=CheckGameID(strGameID);
		//如果没意外情况,5分钟验证一次
		if (nRet == 0 && nCheckGmaeIDRet == 0)
			::InterlockedExchange(&GameMap.CrackCount, 2000);
		Sleep(1000 * 60 * 4);
	}
#endif
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif
	return 0;
}
#define _StartCheck GGGGGG
extern "C"__declspec(dllexport) int WINAPI  _StartCheck(wchar_t* uid, wchar_t* pwd, int nTemp)
{
#ifdef ENCRYPT_VMP
	VMProtectBegin("Check");
#endif
//	char szUID[42] = { 0 };
//	char szPWD[22] = { 0 };
#ifdef NDEBUG
	memset(townmap.szUID, 0, 42);
	memset(townmap.szPWD, 0, 22);
	wcstombs(townmap.szUID, uid, 42);
	wcstombs(townmap.szPWD, pwd, 22);
//	MessageBox(NULL, strUid, strPWD, MB_OK);
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, NULL, CheckThread, NULL, 0, 0);
	CloseHandle(hThread);
#endif
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif
	return 0;
}

extern "C" __declspec(dllexport) int WINAPI  CheckUID(wchar_t* uid, wchar_t* pwd)
{
#ifdef ENCRYPT_VMP
	VMProtectBegin("CheckUID");
#endif
#ifdef NDEBUG
	IOCPCheckClient client;
	char szUID[42] = { 0 };
	char szPWD[22] = { 0 };
	wcstombs(szUID, uid,42);
	wcstombs(szPWD, pwd, 22);
	return client.CheckA(szUID,szPWD);
#else
	return 0;
#endif
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif

}



//function FZ_ReadGarenaAccountBase(PID: ULONG) : ULONG; stdcall;

extern "C" __declspec(dllexport) int WINAPI  GetGGPid(char* UID)
{
	EnablePrivilege(SE_DEBUG_NAME, TRUE);
	DWORD dwPid = 0;
	GetUninjectPid(_T("GarenaMessenger.exe"), &dwPid);
	if (dwPid == 0)
		return 1;
	HMODULE hDll = LoadLibraryW(_T("mem.dll"));
	if (hDll == NULL)
		return 2;
	ReadGarenaAccountBase = (FP_ReadGarenaAccountBase)GetProcAddress(hDll, "MapFile");
	DWORD dwAddr = ReadGarenaAccountBase(dwPid, 11);
	DWORD read;
	dwAddr += 4;
	//char buff[32] = { 0 };
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, dwPid);
	DWORD dwErr = GetLastError();
	
	if (hProcess == NULL)
		return 3;
	BOOL bRet = ReadProcessMemory(hProcess, (LPCVOID)dwAddr, UID, 32, &read);
	/*char buff[64] = { 0 };
	sprintf(buff, "lastError:%d,hProcess:%d,readRet:%d", dwErr, hProcess, bRet);
	MessageBoxA(NULL, buff, NULL, MB_OK);*/
	return 0;
}

//调用方式 EnablePrivilege(SE_DEBUG_NAME,TRUE);
BOOL EnablePrivilege(LPCWSTR lpName, BOOL fEnable)
{
	HANDLE hObject;
	LUID Luid;
	TOKEN_PRIVILEGES NewStatus;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hObject))
		return FALSE;

	if (LookupPrivilegeValue(NULL, lpName, &Luid))
	{
		NewStatus.Privileges[0].Luid = Luid;
		NewStatus.PrivilegeCount = 1;
		NewStatus.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

		AdjustTokenPrivileges(hObject, FALSE, &NewStatus, 0, 0, 0);

		CloseHandle(hObject);
		return TRUE;
	}

	return FALSE;
}


	


//连接外挂
extern "C" __declspec(dllexport) int WINAPI  HelloKitty()
{
	GameMap.Connect();
	townmap.Connect();
	PollutantMap.Connect();
	return 0;
}
#include <tlhelp32.h>
#include <string>
#include "Injector.h"
#include <atlstr.h>
extern "C" __declspec(dllexport) int WINAPI  HelloKitty2()
{
	EnablePrivilege(SE_DEBUG_NAME, TRUE);
	DWORD dwPid = 0;
	for (int i = 0; i < 10; ++i)
	{
		//GetUninjectPid(_T("Client.exe"),&dwPid);
		GetUninjectPid(_T("PathOfExile.exe"), &dwPid);
		if (dwPid)
			break;
		Sleep(500);
	}
	if (dwPid == 0)
		return 1000;

	char szFilePath[MAX_PATH + 1];
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	PathRemoveFileSpecA(szFilePath);
	CStringA strPath = szFilePath;
	strPath += "\\Worker.dll";
	DWORD dwRet = InjectTheDll(dwPid, strPath.GetBuffer());
	return dwRet;
}