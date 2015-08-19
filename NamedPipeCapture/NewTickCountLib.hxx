#ifndef MYLIB_HXX 
#define MYLIB_HXX

#ifndef _WINDOWS_H_INCL
#define _WINDOWS_H_INCL
#include <windows.h> // for DWORD typedefs
#endif

///////////////////////////////////////////////////////////////////////////////

extern "C"
{
__declspec(dllimport) VOID __cdecl AdjustTickCount(DWORD iAdjustAmount);
__declspec(dllimport) DWORD __cdecl SetTickCount(DWORD *pDWNewTickCount);
//__declspec(dllimport) DWORD __cdecl UnloadAutomatically(DWORD *pDWUnloadMS);

__declspec(dllexport) BOOL _cdecl ReadFile(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
	);

_declspec(dllexport) BOOL _cdecl WriteFile(
	HANDLE       hFile,
	LPCVOID      lpBuffer,
	DWORD        nNumberOfBytesToWrite,
	LPDWORD      lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped
	);

_declspec(dllexport) BOOL _cdecl GetQueuedCompletionStatus(
	HANDLE       CompletionPort,
	LPDWORD      lpNumberOfBytes,
	PULONG_PTR   lpCompletionKey,
	LPOVERLAPPED *lpOverlapped,
	DWORD        dwMilliseconds
	);

_declspec(dllexport) BOOL _cdecl CloseHandle(
	HANDLE hObject
	);

_declspec(dllexport) HANDLE _cdecl CreateFile(
	LPCTSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);
}

#endif // #MYLIB_HXX