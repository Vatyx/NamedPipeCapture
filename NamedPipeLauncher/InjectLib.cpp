/******************************************************************************
Module:  InjLib.cpp
Notices: Copyright (c) 2008 Jeffrey Richter & Christophe Nasarre

Modified by M. Scott Mueller for asm calling of library functions
******************************************************************************/

#include "Injlib.hxx"
#include <stdio.h>
#include <malloc.h>        // For alloca
#include <TlHelp32.h>
#include <StrSafe.h>
#include <iostream>



///////////////////////////////////////////////////////////////////////////////


#ifdef UNICODE
#define InjectLib InjectLibW
#define EjectLib  EjectLibW
#else
#define InjectLib InjectLibA
#define EjectLib  EjectLibA
#endif   // !UNICODE


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI InjectLibW(DWORD dwProcessId, PCWSTR pszLibFile) {

	BOOL bOk = FALSE; // Assume that the function fails
	HANDLE hProcess = NULL, hThread = NULL;
	PWSTR pszLibFileRemote = NULL;

	__try {
		// Get a handle for the target process.
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |   // Required by Alpha
			PROCESS_CREATE_THREAD |   // For CreateRemoteThread
			PROCESS_VM_OPERATION |   // For VirtualAllocEx/VirtualFreeEx
			PROCESS_VM_WRITE |             // For WriteProcessMemory
			PROCESS_VM_READ,
			FALSE, dwProcessId);
		if (hProcess == NULL) __leave;


		// Calculate the number of bytes needed for the DLL's pathname
		int cch = 1 + lstrlenW(pszLibFile);
		int cb = cch * sizeof(wchar_t);

		// Allocate space in the remote process for the pathname
		pszLibFileRemote = (PWSTR)
			VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
		if (pszLibFileRemote == NULL) __leave;

		// Copy the DLL's pathname to the remote process' address space
		if (!WriteProcessMemory(hProcess, pszLibFileRemote,
			(PVOID)pszLibFile, cb, NULL)) __leave;

		// Get the real address of LoadLibraryW in Kernel32.dll
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
		if (pfnThreadRtn == NULL) __leave;

		// Create a remote thread that calls LoadLibraryW(DLLPathname)
		hThread = CreateRemoteThread(hProcess, NULL, 0,
			pfnThreadRtn, pszLibFileRemote, 0, NULL);
		if (hThread == NULL)
		{
			DWORD dwerror = 0;
			dwerror = ::GetLastError();
			__leave;
		}

		// Wait for the remote thread to terminate
		WaitForSingleObject(hThread, INFINITE);

		bOk = TRUE; // Everything executed successfully
	}
	__finally { // Now, we can clean everything up

				// Free the remote memory that contained the DLL's pathname
		if (pszLibFileRemote != NULL)
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if (hThread != NULL)
			CloseHandle(hThread);

		if (hProcess != NULL)
			CloseHandle(hProcess);
	}

	return(bOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI InjectLibA(DWORD dwProcessId, PCSTR pszLibFile) {

	// Allocate a (stack) buffer for the Unicode version of the pathname
	SIZE_T cchSize = lstrlenA(pszLibFile) + 1;
	PWSTR pszLibFileW = (PWSTR)
		_alloca(cchSize * sizeof(wchar_t));

	// Convert the ANSI pathname to its Unicode equivalent
	StringCchPrintfW(pszLibFileW, cchSize, L"%S", pszLibFile);

	// Call the Unicode version of the function to actually do the work.
	return(InjectLibW(dwProcessId, pszLibFileW));
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI EjectLibW(DWORD dwProcessId, PCWSTR pszLibFile) {

	BOOL bOk = FALSE; // Assume that the function fails
	HANDLE hthSnapshot = NULL;
	HANDLE hProcess = NULL, hThread = NULL;

	__try {
		// Grab a new snapshot of the process
		hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
		if (hthSnapshot == INVALID_HANDLE_VALUE) __leave;

		// Get the HMODULE of the desired library
		MODULEENTRY32W me = { sizeof(me) };
		BOOL bFound = FALSE;
		BOOL bMoreMods = Module32FirstW(hthSnapshot, &me);
		for (; bMoreMods; bMoreMods = Module32NextW(hthSnapshot, &me)) {
			bFound = (_wcsicmp(me.szModule, pszLibFile) == 0) ||
				(_wcsicmp(me.szExePath, pszLibFile) == 0);
			if (bFound) break;
		}
		if (!bFound) __leave;

		// Get a handle for the target process.
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION |
			PROCESS_VM_WRITE |
			PROCESS_VM_READ,  // For CreateRemoteThread
			FALSE, dwProcessId);
		if (hProcess == NULL) __leave;

		// Get the real address of FreeLibrary in Kernel32.dll
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
		if (pfnThreadRtn == NULL) __leave;

		// Create a remote thread that calls FreeLibrary()
		hThread = CreateRemoteThread(hProcess, NULL, 0,
			pfnThreadRtn, me.modBaseAddr, 0, NULL);
		if (hThread == NULL) __leave;

		// Wait for the remote thread to terminate
		WaitForSingleObject(hThread, INFINITE);

		bOk = TRUE; // Everything executed successfully
	}
	__finally { // Now we can clean everything up

		if (hthSnapshot != NULL)
			CloseHandle(hthSnapshot);

		if (hThread != NULL)
			CloseHandle(hThread);

		if (hProcess != NULL)
			CloseHandle(hProcess);
	}

	return(bOk);
}


///////////////////////////////////////////////////////////////////////////////


BOOL WINAPI EjectLibA(DWORD dwProcessId, PCSTR pszLibFile) {

	// Allocate a (stack) buffer for the Unicode version of the pathname
	SIZE_T cchSize = lstrlenA(pszLibFile) + 1;
	PWSTR pszLibFileW = (PWSTR)
		_alloca(cchSize * sizeof(wchar_t));

	// Convert the ANSI pathname to its Unicode equivalent
	StringCchPrintfW(pszLibFileW, cchSize, L"%S", pszLibFile);

	// Call the Unicode version of the function to actually do the work.
	return(EjectLibW(dwProcessId, pszLibFileW));
}

struct InputStruct
{
	char target[MAX_PATH];
	char output[MAX_PATH];
	unsigned short clientPort;
	unsigned short serverPort;
};

template<typename DATATYPE>
struct DataPage
{
	HMODULE(WINAPI *pGetModuleHandleA)(LPCSTR val);
	DWORD(WINAPI *pGetLastError)(void);
	FARPROC(WINAPI *pGetProcAddress)(HMODULE mod, LPCSTR procname);
	DATATYPE data;
};

DWORD arbitrarycodecall(LPVOID pvoid)
{
	INJPARAM<DataPage<InputStruct>>* LocalParam = reinterpret_cast<INJPARAM<DataPage<InputStruct>>*>(pvoid);
	if (LocalParam == NULL)
		return -1;

	HMODULE hDLL = NULL;
	//hDLL = ::GetModuleHandleA(LocalParam->libname);
	hDLL = LocalParam->data.pGetModuleHandleA(LocalParam->libname);
	if (hDLL == NULL)
	{
		//return ::GetLastError();
		return LocalParam->data.pGetLastError();
	}

	DWORD(WINAPI *pMethodToCall)(LPVOID) = (DWORD(WINAPI *)(LPVOID)) LocalParam->data.pGetProcAddress(hDLL, LocalParam->fcnname);
	if (pMethodToCall == NULL)
	{
		//return ::GetLastError();
		return LocalParam->data.pGetLastError();
	}

	return pMethodToCall(&LocalParam->data.data);
}

void arbitrarycodecall_end(void) {}

LPCVOID GetFunctionBody(LPVOID fcnptr)
{
	//#ifndef WIN64
#if ( !defined (_WIN64) && defined( _DEBUG ) ) || ( defined (_WIN64) && defined(_DEBUG) )

	LPCVOID pLocation = NULL;
	// fcnptr takes us to a jump table under this preproc scope. Parse out the jmp instruction, and extract the offset
	unsigned char *cJtest = NULL;
	cJtest = (unsigned char*)fcnptr;
	std::cout << "orig fcn ptr = " << std::hex << fcnptr << std::dec << std::endl;
	if (*cJtest == 0xe9) // jump near instruction
	{
		// we're in business. Let's grab the DWORD offset.
		DWORD* pDword = NULL;
		pDword = (DWORD*)((INT_PTR)cJtest + (INT_PTR)1);
		pLocation = (LPCVOID)((INT_PTR)*pDword + (INT_PTR)pDword + (INT_PTR)4);
	}
	std::cout << "fcnptr = " << std::hex << pLocation << std::dec << std::endl;
	return pLocation;
	//#endif
#else
	//std::cout << "fcnptr = " << std::hex << fcnptr << std::dec << std::endl;
	return fcnptr;
#endif
}

// returns 0 on success, or whatever the called function returns.
// Perhaps caller & callee should return in different parameters:
// e.g. the injection was successful but the call it made was not.
DWORD WINAPI InitializeProcess(DWORD dwProcessId, const std::string& targetPipeName, const std::string& outputPipeName, unsigned short client, unsigned short server) {

	DWORD dwOk = -1; // Assume that the function fails
	HANDLE hProcess = NULL, hThread = NULL;
	LPVOID pCodeCopy = NULL, pDataCopy = NULL;
	INT_PTR iGetModAddress = NULL, iGetProcAddress = NULL, iGetLastErrorAddress = NULL;

	BYTE *pThisCodePtr = NULL;

	__try {
		// Get a handle for the target process.
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION |   // Required by Alpha
			PROCESS_CREATE_THREAD |   // For CreateRemoteThread
			PROCESS_VM_OPERATION |   // For VirtualAllocEx/VirtualFreeEx
			PROCESS_VM_WRITE |             // For WriteProcessMemory
			PROCESS_VM_READ,
			FALSE, dwProcessId);
		if (hProcess == NULL)
		{
			dwOk = ::GetLastError();
			__leave;
		}

		LPCVOID pLocation = NULL;
		LPCVOID pEndLoc = NULL;

		pLocation = GetFunctionBody(arbitrarycodecall);
		if (pLocation == NULL) __leave;

		pEndLoc = GetFunctionBody(arbitrarycodecall_end);
		SIZE_T fcnsize = (INT_PTR)pEndLoc - (INT_PTR)pLocation;

		pCodeCopy = VirtualAllocEx(hProcess, NULL, fcnsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (pCodeCopy == NULL) __leave;

		HMODULE hKernel = GetModuleHandle(TEXT("Kernel32"));
		if (hKernel == NULL) __leave;
		iGetModAddress = (INT_PTR)GetProcAddress(hKernel, "GetModuleHandleA");
		iGetProcAddress = (INT_PTR)GetProcAddress(hKernel, "GetProcAddress");
		iGetLastErrorAddress = (INT_PTR)GetProcAddress(hKernel, "GetLastError");

		if (iGetModAddress == 0 ||
			iGetProcAddress == 0 ||
			iGetLastErrorAddress == 0)
			__leave;

		// Copy the function code into our internal buffer
		pThisCodePtr = new BYTE[fcnsize];
		memcpy((void*)(pThisCodePtr), pLocation, fcnsize);

		// Patch the buffer with the right memory addresses
		// This is different in debug vs. release, since the dword ptr for pvoid is addressed
		// slightly different in these compile modes.
		// TODO: Make the code cave into a static byte array, different on X64 and X86. If you do this,
		// you can reduce from 4 different address writes down to 2.

		// Write out our code cave to the remote process
		if (!WriteProcessMemory(hProcess, pCodeCopy,
			(LPCVOID)(pThisCodePtr), fcnsize, NULL)) __leave;

		AccessParam<DataPage<InputStruct>> LocalParam;

		//INJPARAM<DWORD> LocalParam;

#ifndef WIN64
		memcpy(&LocalParam.libname, "NamedPipeCapture.DLL", sizeof("NamedPipeCapture.DLL"));
#else
		memcpy(&LocalParam.libname, "NamedPipeCapturex64.DLL", sizeof("NamedPipeCapturex64.DLL"));
#endif
		memcpy(&LocalParam.fcnname, "InitializeCaptureMethods", sizeof("InitializeCaptureMethods"));
		strcpy_s(LocalParam.data.data.output, outputPipeName.c_str());
		strcpy_s(LocalParam.data.data.target, targetPipeName.c_str());
		LocalParam.data.data.clientPort = client;
		LocalParam.data.data.serverPort = server;
		LocalParam.data.pGetLastError = reinterpret_cast<decltype(LocalParam.data.pGetLastError)>(iGetLastErrorAddress);
		LocalParam.data.pGetModuleHandleA = reinterpret_cast<decltype(LocalParam.data.pGetModuleHandleA)>(iGetModAddress);
		LocalParam.data.pGetProcAddress = reinterpret_cast<decltype(LocalParam.data.pGetProcAddress)>(iGetProcAddress);

		// Setup our data memory block
		pDataCopy = VirtualAllocEx(hProcess, NULL, sizeof(LocalParam), MEM_COMMIT, PAGE_READWRITE);
		if (pDataCopy == NULL) __leave;

		if (!WriteProcessMemory(hProcess, pDataCopy,
			(LPVOID)&LocalParam.base(), sizeof(LocalParam.base()), NULL)) __leave;

		hThread = CreateRemoteThread(hProcess, NULL, 0,
			(PTHREAD_START_ROUTINE)pCodeCopy, pDataCopy, 0, NULL);

		if (hThread == NULL)
		{
			dwOk = ::GetLastError();
			__leave;
		}

		// Wait for the remote thread to terminate
		WaitForSingleObject(hThread, INFINITE);

		GetExitCodeThread(hThread, &dwOk);
	}
	__finally { // Now, we can clean everything up

				// Free the remote memory that contained the DLL's pathname
		if (pCodeCopy != NULL)
			VirtualFreeEx(hProcess, pCodeCopy, 0, MEM_RELEASE);

		if (pDataCopy != NULL)
			VirtualFreeEx(hProcess, pDataCopy, 0, MEM_RELEASE);

		// close handles to the thread & process

		if (hThread != NULL)
			CloseHandle(hThread);

		if (hProcess != NULL)
			CloseHandle(hProcess);

		if (pThisCodePtr != NULL)
			delete[] pThisCodePtr;

	}

	return(dwOk);
}

DWORD WINAPI CleanUpEverything(DWORD dwProcessId)
{

   DWORD dwOk = -1; // Assume that the function fails
   HANDLE hProcess = NULL, hThread = NULL;
   LPVOID pCodeCopy = NULL, pDataCopy = NULL;
   INT_PTR iGetModAddress = NULL, iGetProcAddress = NULL, iGetLastErrorAddress = NULL;

   BYTE *pThisCodePtr = NULL;

   __try {
      // Get a handle for the target process.
      hProcess = OpenProcess(
         PROCESS_QUERY_INFORMATION |   // Required by Alpha
         PROCESS_CREATE_THREAD |   // For CreateRemoteThread
         PROCESS_VM_OPERATION |   // For VirtualAllocEx/VirtualFreeEx
         PROCESS_VM_WRITE |             // For WriteProcessMemory
         PROCESS_VM_READ,
         FALSE, dwProcessId);
      if (hProcess == NULL)
      {
         dwOk = ::GetLastError();
         __leave;
      }

      LPCVOID pLocation = NULL;
      LPCVOID pEndLoc = NULL;

      pLocation = GetFunctionBody(arbitrarycodecall);
      if (pLocation == NULL) __leave;

      pEndLoc = GetFunctionBody(arbitrarycodecall_end);
      SIZE_T fcnsize = (INT_PTR) pEndLoc - (INT_PTR) pLocation;

      pCodeCopy = VirtualAllocEx(hProcess, NULL, fcnsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
      if (pCodeCopy == NULL) __leave;

      HMODULE hKernel = GetModuleHandle(TEXT("Kernel32"));
      if (hKernel == NULL) __leave;
      iGetModAddress = (INT_PTR) GetProcAddress(hKernel, "GetModuleHandleA");
      iGetProcAddress = (INT_PTR) GetProcAddress(hKernel, "GetProcAddress");
      iGetLastErrorAddress = (INT_PTR) GetProcAddress(hKernel, "GetLastError");

      if (iGetModAddress == 0 ||
         iGetProcAddress == 0 ||
         iGetLastErrorAddress == 0)
         __leave;

      // Copy the function code into our internal buffer
      pThisCodePtr = new BYTE[fcnsize];
      memcpy((void*) (pThisCodePtr), pLocation, fcnsize);

      // Patch the buffer with the right memory addresses
      // This is different in debug vs. release, since the dword ptr for pvoid is addressed
      // slightly different in these compile modes.
      // TODO: Make the code cave into a static byte array, different on X64 and X86. If you do this,
      // you can reduce from 4 different address writes down to 2.

      // Write out our code cave to the remote process
      if (!WriteProcessMemory(hProcess, pCodeCopy,
         (LPCVOID) (pThisCodePtr), fcnsize, NULL)) __leave;

      AccessParam<DataPage<DWORD>> LocalParam;

      //INJPARAM<DWORD> LocalParam;

#ifndef WIN64
      memcpy(&LocalParam.libname, "NamedPipeCapture.DLL", sizeof("NamedPipeCapture.DLL"));
#else
      memcpy(&LocalParam.libname, "NamedPipeCapturex64.DLL", sizeof("NamedPipeCapturex64.DLL"));
#endif
      memcpy(&LocalParam.fcnname, "DoCleanup", sizeof("DoCleanup"));
      LocalParam.data.pGetLastError = reinterpret_cast<decltype(LocalParam.data.pGetLastError)>(iGetLastErrorAddress);
      LocalParam.data.pGetModuleHandleA = reinterpret_cast<decltype(LocalParam.data.pGetModuleHandleA)>(iGetModAddress);
      LocalParam.data.pGetProcAddress = reinterpret_cast<decltype(LocalParam.data.pGetProcAddress)>(iGetProcAddress);

      // Setup our data memory block
      pDataCopy = VirtualAllocEx(hProcess, NULL, sizeof(LocalParam), MEM_COMMIT, PAGE_READWRITE);
      if (pDataCopy == NULL) __leave;

      if (!WriteProcessMemory(hProcess, pDataCopy,
         (LPVOID) &LocalParam.base(), sizeof(LocalParam.base()), NULL)) __leave;

      hThread = CreateRemoteThread(hProcess, NULL, 0,
         (PTHREAD_START_ROUTINE) pCodeCopy, pDataCopy, 0, NULL);

      if (hThread == NULL)
      {
         dwOk = ::GetLastError();
         __leave;
      }

      // Wait for the remote thread to terminate
      WaitForSingleObject(hThread, INFINITE);

      GetExitCodeThread(hThread, &dwOk);
   }
   __finally { // Now, we can clean everything up

               // Free the remote memory that contained the DLL's pathname
      if (pCodeCopy != NULL)
         VirtualFreeEx(hProcess, pCodeCopy, 0, MEM_RELEASE);

      if (pDataCopy != NULL)
         VirtualFreeEx(hProcess, pDataCopy, 0, MEM_RELEASE);

      // close handles to the thread & process

      if (hThread != NULL)
         CloseHandle(hThread);

      if (hProcess != NULL)
         CloseHandle(hProcess);

      if (pThisCodePtr != NULL)
         delete [] pThisCodePtr;

   }

   return(dwOk);
}

//////////////////////////////// End of File //////////////////////////////////
