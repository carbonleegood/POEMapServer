// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <atlstr.h>
#ifdef ENCRYPT_VMP
#include "VMProtectSDK.h"
#endif
HWND hGameWnd;
TCHAR szModulePath[MAX_PATH] = { 0 };
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
#ifdef ENCRYPT_VMP
	VMProtectBegin("MapServerDllMain");
#endif
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		GetModuleFileName(hModule, szModulePath, MAX_PATH);
		PathRemoveFileSpec(szModulePath);
		hGameWnd = ::FindWindow(L"Direct3DWindowClass", NULL);
		break;
		//	MessageBoxW(NULL,L"aaaaa", NULL, MB_OK);
		//	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif
	return TRUE;
}

