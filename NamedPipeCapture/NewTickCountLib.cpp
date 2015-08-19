// MyLib.cpp : Defines the entry point for the DLL application.
//
#define _WIN32_WINNT 0x0501 
#include <windows.h>
#include <process.h>
#include "APIHook.h"

DWORD dwOffset = 0; // additive offset
BOOL hooked = FALSE;
CAPIHook* g_pTickCount = NULL;

DWORD __stdcall MYGetTickCount(VOID);

// forward declarations
extern "C"
{

__declspec(dllexport) VOID __cdecl AdjustTickCount(DWORD iAdjustAmount)
{
	dwOffset = iAdjustAmount;
}

__declspec(dllexport) DWORD __cdecl SetTickCount(DWORD *pDWNewTickCount)
{
   if (!pDWNewTickCount)
      return - 1;

	DWORD dwOldTickCount = ::GetTickCount();
	dwOffset = *pDWNewTickCount - dwOldTickCount;
	return 0;
}

__declspec(dllexport) DWORD __cdecl GetThisTickCount(DWORD* pVal)
{
   if (!pVal)
      return -1;

   *pVal = MYGetTickCount();
   return 0;
}

}

DWORD __stdcall MYGetTickCount(VOID)
{
	DWORD tickcount = ::GetTickCount();	
	return tickcount + dwOffset;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			hooked = true;
			DWORD dwError = 0;
			g_pTickCount = new CAPIHook("Kernel32.dll",
				"GetTickCount", 
				(PROC) MYGetTickCount);
			break;
		}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			if (hooked)
			{
				delete g_pTickCount;
			}
			break;
		}
	}
    return TRUE;
}

extern "C"
{

	__declspec(dllexport) BOOL _cdecl myReadFile(
		HANDLE       hFile,
		LPVOID       lpBuffer,
		DWORD        nNumberOfBytesToRead,
		LPDWORD      lpNumberOfBytesRead,
		LPOVERLAPPED lpOverlapped
		)
	{


	}

	_declspec(dllexport) BOOL _cdecl WriteFile(
		HANDLE       hFile,
		LPCVOID      lpBuffer,
		DWORD        nNumberOfBytesToWrite,
		LPDWORD      lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped
		)
	{

	}

	_declspec(dllexport) BOOL _cdecl GetQueuedCompletionStatus(
		HANDLE       CompletionPort,
		LPDWORD      lpNumberOfBytes,
		PULONG_PTR   lpCompletionKey,
		LPOVERLAPPED *lpOverlapped,
		DWORD        dwMilliseconds
		)
	{

	}

	_declspec(dllexport) BOOL _cdecl CloseHandle(
		HANDLE hObject
		)
	{
		
	}

	_declspec(dllexport) HANDLE _cdecl CreateFile(
		LPCTSTR               lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
		)
	{

	}
}



