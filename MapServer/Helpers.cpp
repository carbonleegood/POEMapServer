#include "StdAfx.h"
#include "Helpers.h"
#include <aclapi.h>
#include <tchar.h>
#include "share.h"

//--------------------------------------------------------------------
// Registry Helpers
//
BOOL _RegReadStringValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szValue, LPTSTR szString, DWORD dwSize)
{
   HKEY hKey;
   BOOL bResult = FALSE;
   if (::RegOpenKeyEx(hRoot, szKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) 
   {
      DWORD dwType = 0;
      DWORD dwMax = dwSize;
      LONG lResult = ::RegQueryValueEx(
         hKey,
         szValue,
         NULL,
         &dwType,
         (BYTE*)szString,
         &dwSize
         );

      bResult = (lResult == ERROR_SUCCESS) && (dwSize < dwMax);
      ::RegCloseKey(hKey);
   }
   
   return(bResult);
}


BOOL _RegWriteStringValue(HKEY hRoot, LPCTSTR szKey, LPCTSTR szValue, LPCTSTR szInfo)
{
// sanity checks
   if (
         (szKey == NULL) ||
         (_tcslen(szKey) == 0) ||
         (szValue == NULL) ||
         (_tcslen(szValue) == 0)
      )
      return(FALSE);


   HKEY hKey;
   BOOL bResult = FALSE;
   DWORD dwDisposition;
   if (::RegOpenKeyEx(hRoot, szKey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) 
   {
   // maybe the key does not exist
   // --> try to create it
      if (
            ::RegCreateKeyEx(
               hRoot,
               szKey,
               0,
               NULL,
               REG_OPTION_NON_VOLATILE,
               KEY_WRITE,
               NULL,
               &hKey,
               &dwDisposition
               )
            != 
            ERROR_SUCCESS
         )
         return(FALSE);
   }

   LONG lResult = 
      ::RegSetValueEx(
         hKey,
         szValue,
         NULL,
         REG_SZ,
         (BYTE*)szInfo,
         (szInfo == NULL) ? 0 : _tcslen(szInfo)+1
         );

   bResult = (lResult == ERROR_SUCCESS);
   ::RegCloseKey(hKey);
   
   return(bResult);
}
//
//--------------------------------------------------------------------


//--------------------------------------------------------------------
// Module related
// 
void TranslateFilename(LPCTSTR szFilename, LPTSTR szWin32Name)
{
// sanity checks
   if (szFilename == NULL)
   {
      szWin32Name = _T("");
      return;
   }
   if (szFilename[0] == _T('\0'))
   {
      szWin32Name = _T("");
      return;
   }

// check for "strange" filenames
   LPCTSTR pszInfo = _tcsstr(szFilename, _T("\\SystemRoot\\"));
   if (pszInfo == szFilename)
   {
   //       \SystemRoot\System32\smss.exe
   // -->   c:\winnt\System32\smss.exe  using GetWindowsDirectory()
      UINT Len = ::GetWindowsDirectory(szWin32Name, MAX_PATH);
      if (Len != 0)
      {
         _tcscat(szWin32Name, _T("\\"));
         _tcscat(szWin32Name, &szFilename[_tcslen(_T("\\SystemRoot\\"))]);

         return;
      }
   }
   else
   {
   //       \??\C:\WINNT\system32\winlogon.exe
   // -->   C:\WINNT\system32\winlogon.exe  
      pszInfo = _tcsstr(szFilename, _T("\\??\\"));
      if (pszInfo == szFilename)
      {
         _tcscpy(szWin32Name, &szFilename[_tcslen(_T("\\??\\"))]);
      }
      else
         _tcscpy(szWin32Name, szFilename);

      return;
   }


// standard name
   _tcscpy(szWin32Name, szFilename);
}
//
//--------------------------------------------------------------------


//--------------------------------------------------------------------
// return the last error (re-set it)
// 
void GetLastErrorMessage(LPTSTR szInfo)
{
   DWORD LastError = ::GetLastError();
   LPVOID lpMessageBuffer; 
   if (
         ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            LastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
            (LPTSTR)&lpMessageBuffer,
            0,
            NULL 
            )
      )
   {
      _tcscpy(szInfo, (LPCTSTR)lpMessageBuffer);
      ::LocalFree(lpMessageBuffer); 
   }
   else
      wsprintf(szInfo, _T("Error: 0x%x"), LastError);

// don't forget to reset current error
   ::SetLastError(LastError);
}
//
//--------------------------------------------------------------------



//--------------------------------------------------------------------
// comparison helpers
//
int Compare64Bits(DWORD MS1, DWORD LS1, DWORD MS2, DWORD LS2)
{
   __int64 i1 = (__int64)MS1 * (__int64)0x100000000 + (__int64)LS1;
   __int64 i2 = (__int64)MS2 * (__int64)0x100000000 + (__int64)LS2;
   return(
      ((i1 - i2) > 0) ? 1 : 
      (i1 == i2) ? 0 :
      -1
      );

// first compare most significant bits
   if (MS1 > MS2)
      return(1);
   else
   {
      if (MS1 < MS2)
         return(-1);
      else
      {
      // then, least significant bits
         return(LS1 - LS2);
      }
   }
}
//
//--------------------------------------------------------------------



//--------------------------------------------------------------------
// Format a number according to a pattern [xxSEPARATORxxxSEPARATORxxxSEPARATORxxx]:
//    1000000, '.', 3  --> "1.000.000"                 ^-- GroupLength=3
//    1000000, '-', 2  --> "1-00-00-00"
// 
// Build the string from the end and reverse it when it is finished
// Here is the state of the two variables at the end of each iteration
//    dwNumber    szBuffer
//      1000000           
//      100000           0
//       10000          00
//        1000        000.
//         100       000.0
//          10      000.00
//           1    000.000.
//           0   000.000.1
//
void FormatNumber(DWORD dwNumber, LPTSTR szNumber, TCHAR cSeparator, DWORD GroupLength)
{
   LPTSTR   pFirst      = szNumber;
   LPTSTR   pChar       = pFirst;
   DWORD    DigitCount  = 0;
   //DWORD    wNumChars   = 0;

   while (dwNumber > 0)
   {
   // extract the last digit
      wsprintf(pChar, _T("%u"), dwNumber % 10);

   // next character in the buffer
      pChar++;

   // next digit in the current group
      DigitCount++;

   // eat the last digit
      dwNumber = dwNumber / 10;

   // add the separator if needed
      if ((DigitCount == GroupLength) && (dwNumber > 0))
      {
         *pChar = cSeparator;

      // begin a new digit group after the separator
         DigitCount = 0;
         pChar++;
      }
   };

// close the string
   *pChar = _T('\0');

// reverse it
   _tcsrev(pFirst);
}
//
//--------------------------------------------------------------------



//--------------------------------------------------------------------
// timer/date helpers from Matt Pietrek's DEPENDS (MSJ February 1997)
//

// Convert a TimeDateStamp (i.e., # of seconds since 1/1/1970) into a FILETIME
BOOL TimeDateStampToFileTime(DWORD timeDateStamp, LPFILETIME pFileTime)
{
   __int64 t1970 = 0x019DB1DED53E8000; // Magic... GMT...  Don't ask....

   __int64 timeStampIn100nsIncr = (__int64)timeDateStamp * 10000000;

   __int64 finalValue = t1970 + timeStampIn100nsIncr;

   memcpy( pFileTime, &finalValue, sizeof( finalValue ) );

   return(TRUE);
}


BOOL GetFileDateAsString(LPFILETIME pFt, LPTSTR pszDate, unsigned cbIn)
{
   FILETIME ftLocal;
   SYSTEMTIME st;

   if (!FileTimeToLocalFileTime(pFt, &ftLocal))
      return(FALSE);

   if (!FileTimeToSystemTime(&ftLocal, &st))
      return(FALSE);

   TCHAR szTemp[12];

   wsprintf(
      szTemp, 
      _T("%02u/%02u/%04u"),
      st.wMonth, st.wDay, st.wYear 
      );
   lstrcpyn(pszDate, szTemp, cbIn);

   return(TRUE);
}


BOOL GetFileTimeAsString(LPFILETIME pFt, LPTSTR pszTime, unsigned cbIn, BOOL fSeconds)
{
   FILETIME ftLocal;
   SYSTEMTIME st;

   if (!FileTimeToLocalFileTime(pFt, &ftLocal))
      return(FALSE);

   if (!FileTimeToSystemTime(&ftLocal, &st))
      return(FALSE);

   TCHAR szTemp[12];

   if (fSeconds)  // Want seconds???
   {
      wsprintf(szTemp, _T("%02u:%02u:%02u"), st.wHour, st.wMinute, st.wSecond);
   }
   else           // No thanks..  Just hours and minutes
   {
      wsprintf(szTemp, _T("%02u:%02u"), st.wHour, st.wMinute);
   }

   lstrcpyn(pszTime, szTemp, cbIn);

   return(TRUE);
}
//
//--------------------------------------------------------------------



//--------------------------------------------------------------------
// based on Keith Brown (MSJ August 1999 column)
// 
BOOL EnablePrivilege(LPCTSTR szPrivilege)
{
   BOOL bReturn = FALSE;

   HANDLE hToken;
   if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
      return(FALSE);

   TOKEN_PRIVILEGES tpOld;
   bReturn = (EnableTokenPrivilege(hToken, szPrivilege, tpOld));

// don't forget to close the token handle
   ::CloseHandle(hToken);

   return(bReturn);
}


BOOL AdjustDacl(HANDLE h, DWORD DesiredAccess)
{
// the WORLD Sid is trivial to form programmatically (S-1-1-0)
   SID world = { SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, 0 };

   EXPLICIT_ACCESS ea =
   {
      DesiredAccess,
      SET_ACCESS,
      NO_INHERITANCE,
      {
         0, NO_MULTIPLE_TRUSTEE,
         TRUSTEE_IS_SID,
         TRUSTEE_IS_USER,
         reinterpret_cast<LPTSTR>(&world)
      }
   };
   ACL* pdacl = 0;
   DWORD err = SetEntriesInAcl(1, &ea, 0, &pdacl);
   if (err == ERROR_SUCCESS)
   {
      err = SetSecurityInfo(h, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, pdacl, 0);
      LocalFree(pdacl);
      return(err == ERROR_SUCCESS);
   }
   else
      return(FALSE);
}


// Useful helper function for enabling a single privilege
BOOL EnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES& tpOld)
{
   TOKEN_PRIVILEGES tp;
   tp.PrivilegeCount = 1;
   tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
   if (LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
   {
   // htok must have been opened with the following permissions:
   // TOKEN_QUERY (to get the old priv setting)
   // TOKEN_ADJUST_PRIVILEGES (to adjust the priv)
      DWORD cbOld = sizeof tpOld;
      if (AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, &tpOld, &cbOld))
      // Note that AdjustTokenPrivileges may succeed, and yet
      // some privileges weren't actually adjusted.
      // You've got to check GetLastError() to be sure!
         return(ERROR_NOT_ALL_ASSIGNED != GetLastError());
      else
         return(FALSE);
   }
   else
      return(FALSE);
}


// Corresponding restoration helper function
BOOL RestoreTokenPrivilege(HANDLE htok, const TOKEN_PRIVILEGES& tpOld)
{
   return(AdjustTokenPrivileges(htok, FALSE, const_cast<TOKEN_PRIVILEGES*>(&tpOld), 0, 0, 0));
}


HANDLE GetProcessHandleWithEnoughRights(DWORD PID, DWORD AccessRights)
{
   HANDLE hProcess = ::OpenProcess(AccessRights, FALSE, PID);
   if (hProcess == NULL)
   {
      HANDLE hpWriteDAC = OpenProcess(WRITE_DAC, FALSE, PID);
      if (hpWriteDAC == NULL)
      {
      // hmm, we don't have permissions to modify the DACL...
      // time to take ownership...
         HANDLE htok;
         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &htok))
            return(FALSE);

         TOKEN_PRIVILEGES tpOld;
         if (EnableTokenPrivilege(htok, SE_TAKE_OWNERSHIP_NAME, tpOld))
         {
         // SeTakeOwnershipPrivilege allows us to open objects with
         // WRITE_OWNER, but that's about it, so we'll update the owner,
         // and dup the handle so we can get WRITE_DAC permissions.
            HANDLE hpWriteOwner = OpenProcess(WRITE_OWNER, FALSE, PID);
            if (hpWriteOwner != NULL)
            {
               BYTE buf[512]; // this should always be big enough
               DWORD cb = sizeof buf;
               if (GetTokenInformation(htok, TokenUser, buf, cb, &cb))
               {
                  DWORD err = 
                     SetSecurityInfo( 
                        hpWriteOwner, 
                        SE_KERNEL_OBJECT,
                        OWNER_SECURITY_INFORMATION,
                        reinterpret_cast<TOKEN_USER*>(buf)->User.Sid,
                        0, 0, 0 
                        );
                  if (err == ERROR_SUCCESS)
                  {
                  // now that we're the owner, we've implicitly got WRITE_DAC
                  // permissions, so ask the system to reevaluate our request,
                  // giving us a handle with WRITE_DAC permissions
                     if (
                           !DuplicateHandle( 
                              GetCurrentProcess(), 
                              hpWriteOwner,
                              GetCurrentProcess(), 
                              &hpWriteDAC,
                              WRITE_DAC, FALSE, 0 
                              ) 
                        )
                     hpWriteDAC = NULL;
                  }
               }

            // don't forget to close handle
               ::CloseHandle(hpWriteOwner);
            }

         // not truly necessary in this app,
         // but included for completeness
            RestoreTokenPrivilege(htok, tpOld);
         }

      // don't forget to close the token handle
         ::CloseHandle(htok);
      }

      if (hpWriteDAC)
      {
      // we've now got a handle that allows us WRITE_DAC permission
         AdjustDacl(hpWriteDAC, AccessRights);

      // now that we've granted ourselves permission to access 
      // the process, ask the system to reevaluate our request,
      // giving us a handle with right permissions
         if (
               !DuplicateHandle( 
                  GetCurrentProcess(), 
                  hpWriteDAC,
                  GetCurrentProcess(), 
                  &hProcess,
                  AccessRights, 
                  FALSE, 
                  0 
                  ) 
               )
            hProcess = NULL;

         CloseHandle(hpWriteDAC);
      }
   }

   return(hProcess);
}


BOOL SlayProcess(DWORD PID)
{
   HANDLE hp = GetProcessHandleWithEnoughRights(PID, PROCESS_TERMINATE);
   if (hp != NULL)
   {
   // if all went well, we've now got a handle to the process
   // that grants us PROCESS_TERMINATE permissions
      BOOL bReturn = TerminateProcess(hp, 1);

   // don't forget to close process handle
      ::CloseHandle(hp);

      return(bReturn);
   }

   return(FALSE);
}
//
//--------------------------------------------------------------------
