#include "StdAfx.h"
#include "Injector.h"
///#include "MemIPCDefine.h"
#include "tlhelp32.h"
#include <tchar.h>
#include <aclapi.h>
#include "Helpers.h"
//��������
//������ ������ �����ҵ���ID����
DWORD SearchTargetProcess(TCHAR* szProcessName,DWORD dwPid[])
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;
	//TCHAR* p=_T("notepad.exe");
	//TCHAR* p=_T("rift.exe");
	TCHAR* p=szProcessName;
    int i=0;
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{

		return FALSE;
	}
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );          // clean the snapshot object
		return( FALSE );
	}
	
	do
	{
		if(i>100)
			break;
		if(0==_tcsicmp(p,pe32.szExeFile))
		{
			/*_tprintf( TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile );
			_tprintf( TEXT("\n  Process ID        = %d"), pe32.th32ProcessID );*/
			dwPid[i]=pe32.th32ProcessID;
			i++;
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );
	CloseHandle( hProcessSnap );
	
	return RT_SUCCESS ;
}
typedef DWORD (WINAPI *PFNTCREATETHREADEX)  
(   
    PHANDLE                 ThreadHandle,     
    ACCESS_MASK             DesiredAccess,    
    LPVOID                  ObjectAttributes,     
    HANDLE                  ProcessHandle,    
    LPTHREAD_START_ROUTINE  lpStartAddress,   
    LPVOID                  lpParameter,      
    BOOL                    CreateSuspended,      
    DWORD                   dwStackSize,      
    DWORD                   dw1,   
    DWORD                   dw2,   
    LPVOID                  Unknown   
);

BOOL MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf)
{
	HANDLE      hThread = NULL;  
	FARPROC     pFunc = NULL;  

	pFunc = GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtCreateThreadEx");  
	if( pFunc == NULL )  
	{  
//		printf("MyCreateRemoteThread() : GetProcAddress(\"NtCreateThreadEx\") ����ʧ�ܣ��������: [%d]/n",  
//			GetLastError());  
		return FALSE;  
	}  
	((PFNTCREATETHREADEX)pFunc)(&hThread,  
		0x1FFFFF,  
		NULL,  
		hProcess,  
		pThreadProc,  
		pRemoteBuf,  
		FALSE,  
		NULL,  
		NULL,  
		NULL,  
		NULL);  
	if( hThread == NULL )  
	{  
//		printf("MyCreateRemoteThread() : NtCreateThreadEx() ����ʧ�ܣ��������: [%d]/n", GetLastError());  
		return FALSE;  
	}  
	return TRUE;
}
DWORD InjectTheDll(DWORD dwPid,char* szDllName)
{
	HANDLE hRemoteProcess;
	if((hRemoteProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwPid))==NULL)
	{
//		printf("OpenProcess error\n");
		return 1;
	}
	char *pszLibFileRemote;
	//������dll�ļ�����·��
	pszLibFileRemote=(char *)VirtualAllocEx( hRemoteProcess,NULL,strlen(szDllName)+1,MEM_COMMIT, PAGE_READWRITE);
	if(pszLibFileRemote==NULL)
	{
//		printf("VirtualAllocEx error\n");
		return 2;
	}
	DWORD dwWrite=0;
	//��dll������·��д�뵽�ڴ棬
	if(WriteProcessMemory(hRemoteProcess,pszLibFileRemote,(void *)szDllName,strlen(szDllName)+1,&dwWrite) == 0)
	{
//		printf("WriteProcessMemory error\n");
		return 3;
	}
//	printf("д����%d�ֽ�",dwWrite);
	//�õ�LoadLibraryA������ַ
	PTHREAD_START_ROUTINE pfnStartAddr=(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")),"LoadLibraryA");
	if(pfnStartAddr == NULL)
	{
//		printf("GetProcAddress error\n");
		return 4;
	}
	/*DWORD read=0;
	char temp[32]={0};
	if(ReadProcessMemory(hRemoteProcess,pfnStartAddr,temp,16,&read)==FALSE)
	{
		printf("������%d",GetLastError());
	}*/
	/*{
		for(int i=0;i<8;i++)
		{
			printf("0x%x\n",*(temp+i*4));
		}
	}*/
	HANDLE hRemoteThread;
	//����Զ���߳�
	//hRemoteProcess=(HANDLE)0x8c;
	/*if(MyCreateRemoteThread(hRemoteProcess,pfnStartAddr,pszLibFileRemote)==FALSE)
	{
		printf("����ʧ��");
		return FALSE;
	}*/
	if( (hRemoteThread = CreateRemoteThread(hRemoteProcess,NULL,0,pfnStartAddr,pszLibFileRemote,0,NULL))==NULL)
	{
		DWORD err=GetLastError();
//		printf("CreateRemoteThread error%d\n",err);
		return 5;
	}
	return 0;
}

//ע��DLL
//DWORD InjectTheDll(DWORD dwPid,TCHAR* szDllName)
//{
//	/*if (HasAttachDll(processId,dllName))
//		return FALSE; */
//	OutputDebugStringA("׼��ע��");
//	DWORD dwResult = FALSE;
//	HANDLE hProcess = NULL;
//	HANDLE hThread = NULL;
//	PWSTR libFileRemote = NULL;
//	do 
//	{
//		//. ��ý��̾��
//		hProcess = GetProcessHandleWithEnoughRights(dwPid, PROCESS_ALL_ACCESS);
//	 //   hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
//	//	hProcess = GetProcessHandleWithEnoughRights(dwPid, PROCESS_ALL_ACCESS);
//		if (hProcess == NULL) 
//			break;
//
//		OutputDebugStringA("a1");
//		//.��Զ�̽�����ΪDLL�ļ��������ڴ�
//		int cb  = int(::_tcslen (szDllName)+1) * sizeof(WCHAR);
//		libFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
//		if (libFileRemote == NULL) 
//			break;
//		OutputDebugStringA("a2");
//		//. ��DLL���ļ�����ȫ·����д��������ڴ���
//		if (!WriteProcessMemory(hProcess, libFileRemote, szDllName, cb, 0))
//			break;
//		OutputDebugStringA("a3");
//		//. ӳ���Զ�̽���
//		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
//		if (pfnThreadRtn == NULL)  
//			break;
//
//		OutputDebugStringA("a4");
//		//. �ȴ�Զ���߳̽���, ��DllMain����
//		/*if(MyCreateRemoteThread(hProcess,pfnThreadRtn,libFileRemote)==FALSE)
//		{
//			printf("����ʧ��");
//			return FALSE;
//		}*/
//		DWORD dwID=0;
//		hThread = ::CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, (LPVOID)libFileRemote, 0, &dwID);
//		if (hThread == NULL) 
//			break;
//		OutputDebugStringA("a5");
//		WaitForSingleObject(hThread, INFINITE);
//	} 
//	while(0);
//
//	if (libFileRemote != NULL)
//		VirtualFreeEx(hProcess, libFileRemote, 0, MEM_RELEASE);
//	if (hThread != NULL)
//		CloseHandle(hThread);
//	if (hProcess != NULL)
//		CloseHandle(hProcess);
//	
//	return dwResult;
//}
//�Ƿ��Ѿ�ע���
//DWORD IsInjected(DWORD dwPid,TCHAR* szDllName)
//{
////���ڸ����Ƿ�����صĹ����ڴ�ͻ������жϣ�Ҳ����ͨ��DLL���ж�
//    DWORD dwResult=RT_NO_RESULT;
//	TCHAR szShareMemString[KEY_STRING_LENGTH]={0};
//	_stprintf(szShareMemString,SHARE_MEM_STRING,dwPid);
//	TCHAR szMutexString[KEY_STRING_LENGTH]={0};
//	_stprintf(szMutexString,MUTEX_STRING,dwPid);
//	//���Դ򿪹����ڴ�
//	do
//	{
//		HANDLE hShareMem=OpenFileMapping(FILE_MAP_ALL_ACCESS,false,szShareMemString);
//
//		if(NULL!=hShareMem)
//		{
//			dwResult=RT_INJECTED;
//			break;
//		}
//		//������
//		HANDLE hMutex=OpenMutex(MUTEX_ALL_ACCESS,false,szMutexString);
//		//����򿪻�����ʧ�ܣ���˵��������������������ϲ㱨�棬��Ϊ��������ڴ棬ȴû��������˵����һ�˴�������
//		if(NULL!=hMutex)
//		{
//			dwResult=RT_INJECTED;
//			break;
//		}
//		dwResult=RT_UNINJECTED;
//	}while(0);
//	return dwResult;
//}

//��ȡһ����ע�����ID
//���������������һ����ע��Ľ���ID
DWORD GetUninjectPid(TCHAR* szProcessName,DWORD* dwPid)
{
	DWORD dwResult=0;
	DWORD dwPidArray[500]={0};
	//�������н���ID
    SearchTargetProcess(szProcessName,dwPidArray);

	//���˵�ע�����
	for(int i=0;i<500;i++)
	{
		//if(RT_UNINJECTED==IsInjected(dwPidArray[i],L"UnUseful"))//��ʱû��DLL���ж��Ƿ�ע����������ù����ڴ���
		if (dwPidArray[i]!=0)
		{
			*dwPid=dwPidArray[i];
			break;
		}
	}
	//����ֱ��ע��
	return dwResult;
}

