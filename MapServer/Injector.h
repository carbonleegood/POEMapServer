#pragma once
#include "windef.h"
DWORD SearchTargetProcess(TCHAR* szProcessName,DWORD dwPid[]);
//DWORD InjectTheDll(DWORD dwPid,TCHAR* szDllName);
DWORD InjectTheDll(DWORD dwPid,char* szDllName);
DWORD IsInjected(DWORD dwPid,TCHAR* szDllName);
DWORD GetUninjectPid(TCHAR* szProcessName,DWORD* dwPid);