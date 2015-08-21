// Header for InjLib, 2008 by Jeffrey Richter and Christopher Nasarre
// Modified by M. Scott Mueller for dynamic function loading

#ifndef _INJLIB_HXX_INCL_
#define _INJLIB_HXX_INCL_


#include "CmnHdr.h"     /* See Appendix A. */

#ifndef _WINDOWSX_H_INCL
#define _WINDOWSX_H_INCL
#include <windowsx.h> // for various defs
#endif

#ifndef _TCHAR_H_INCL_
#define _TCHAR_H_INCL_
#include <tchar.h>
#endif

#include <string>

#ifdef UNICODE
   #define InjectLib InjectLibW
   #define EjectLib  EjectLibW
#else
   #define InjectLib InjectLibA
   #define EjectLib  EjectLibA
#endif   // !UNICODE

BOOL WINAPI InjectLibW(DWORD dwProcessId, PCWSTR pszLibFile);
BOOL WINAPI InjectLibA(DWORD dwProcessId, PCSTR pszLibFile);
BOOL WINAPI EjectLibW(DWORD dwProcessId, PCWSTR pszLibFile);
BOOL WINAPI EjectLibA(DWORD dwProcessId, PCSTR pszLibFile);
DWORD WINAPI InitializeProcess(DWORD dwProcessId, const std::string& targetPipeName, 
	const std::string& outputPipeName, unsigned short clientPort, unsigned short serverPort);

// Todo: add a field that tells us how large the BYTE array is after the char arrays
// Note that the DATACLASS must be a built-in type or a self-contained struct.
// All reference types must offset from the start of the object, as the object
// must be portable across process spaces.
template<typename DATACLASS>
struct INJPARAM
{
   static_assert(std::is_pod<DATACLASS>::value == true, "A POD type is required as the parameter");
	CHAR libname[1024];
	CHAR fcnname[1024];
	//int m_iDataSize;
	DATACLASS data;
	
   INJPARAM()
	{
		strcpy_s((CHAR*)&libname, 1024, "\0");
		strcpy_s((CHAR*)&fcnname, 1024, "\0");
	}
};

// This is a bit of syntactic sugar to keep the code looking a bit more clean. NOTE:
// you must pass &AccessParam.base() to the code cave, not this class.
// However, you can just do the cast to INJPARAM<TYPE> instead, but please use
// a dynamic cast for type safety if you do this.
template <typename TYPE>
struct AccessParam : public INJPARAM<TYPE>
{
   TYPE& ValRef()
   {
      return static_cast<typename INJPARAM<TYPE>*>(this).data;
   }
   const TYPE& ValRef() const
   {
      return const_cast<const TYPE&>(const_cast<AccessParam*>(this).ValRef());
   }
   INJPARAM<TYPE>& base(void) {return dynamic_cast<INJPARAM<TYPE>&>(*this);}
};

template<typename DATACLASS>
DWORD WINAPI tINJECTFUNCTION(LPVOID pParam)
{
	DWORD dwRetVal = 0;
	HMODULE hLibHandle = NULL;
	INJPARAM<DATACLASS>* pInj = NULL;
	pInj = (INJPARAM<DATACLASS>*) pParam;
	__try
	{
		hLibHandle = GetModuleHandle(pInj->libname);
		if (hLibHandle == NULL)
		{
			dwRetVal = ::GetLastError();
			chMB("GetModuleHandle failed.");
			__leave;
		}

		// Get proc address
		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(hLibHandle, (LPCSTR) &pInj->fcnname);
		if (pfnThreadRtn == NULL)
		{
			dwRetVal = ::GetLastError();
			chMB("GetProcAddress failed.");
			__leave;
		}

		// Call proc address
		return (*pfnThreadRtn)(pInj->&data);
	}
	__finally
	{
		if (hLibHandle != NULL)
		{
			CloseHandle(hLibHandle);
		}
	}
	return dwRetVal;
}

#endif // _INJLIB_HXX_INCL_