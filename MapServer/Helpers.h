#ifndef _HELPERS_H
#define _HELPERS_H


// Registry
// 
BOOL _RegReadStringValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szValue, LPTSTR szString, DWORD dwSize);
BOOL _RegWriteStringValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szValue, LPCTSTR szInfo);


// Error
// 
void GetLastErrorMessage(LPTSTR szInfo);


// Formatting 
//
void FormatNumber(DWORD dwNumber, LPTSTR szNumber, TCHAR cSeparator, DWORD GroupLength);
int Compare64Bits(DWORD MS1, DWORD LS1, DWORD MS2, DWORD LS2);


// Time 
//
BOOL TimeDateStampToFileTime(DWORD timeDateStamp, LPFILETIME pFileTime);
BOOL GetFileDateAsString(LPFILETIME pFt, LPTSTR pszDate, unsigned cbIn);
BOOL GetFileTimeAsString(LPFILETIME pFt, LPTSTR pszTime, unsigned cbIn, BOOL fSeconds);


// Process & Security
// 
BOOL EnablePrivilege(LPCTSTR szPrivilege);
BOOL SlayProcess(DWORD PID);
BOOL AdjustDacl(HANDLE h, DWORD DesiredAccess); 
BOOL EnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES& tpOld);
BOOL RestoreTokenPrivilege(HANDLE htok, const TOKEN_PRIVILEGES& tpOld);
HANDLE GetProcessHandleWithEnoughRights(DWORD PID, DWORD AccessRights);


// Module
// 
void TranslateFilename(LPCTSTR szFilename, LPTSTR szWin32Name);


#endif