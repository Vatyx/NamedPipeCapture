#pragma once

#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <locale>
#include <codecvt>
#include <chrono>

#include <boost\asio.hpp>
#include "boost\ref.hpp"

#include "NamedPipeServer.h"
#include "NamedPipe.h"
#include "Streamer.h"
#include "Buffer.h"
#include "Globals.h"
#include "CaptureEngine.h"

#include "APIHook.h"

bool hooked = false;
CAPIHook* g_pReadFile = NULL;
CAPIHook* g_pWriteFile = NULL;
CAPIHook* g_pGetQueuedCompletionStatus = NULL;

enum STATE
{
   WRITE = 1,
   READ = 2
};

globalStruct* GetGlobals() { return globalStruct::GetGlobals(); }

struct InputStruct
{
   char target[MAX_PATH];
   char output[MAX_PATH];
   unsigned short clientPort;
   unsigned short serverPort;
};

bool IsInitializedAndConnected()
{
   auto glbl = GetGlobals();
   if (!glbl)
      return false;
   return std::atomic_load(&glbl->pipe) != nullptr;
}

bool CheckNamedPipeName(HANDLE hFile, const std::string& name)
{
   if (GetFileType(hFile) == FILE_TYPE_PIPE)
   {
      auto sizeOfObject = sizeof(TCHAR) * _MAX_PATH + sizeof(FILE_NAME_INFO);
      std::unique_ptr<char[]> ptrInfo(new char[sizeOfObject]);
      ZeroMemory(ptrInfo.get(), sizeOfObject);
      FILE_NAME_INFO* structInfo = reinterpret_cast<FILE_NAME_INFO*>(ptrInfo.get());

      GetFileInformationByHandleEx(hFile, FileNameInfo, structInfo,
                                   sizeof(structInfo) + (_MAX_PATH * sizeof(TCHAR)));
      std::wstring gottenName(structInfo->FileName, structInfo->FileNameLength);

      std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
      std::string compareString = conv1.to_bytes(gottenName);

      if (std::string(compareString.c_str()) == name)
      {
         return true;
      }
   }

   return false;
}

// See if we're initialized AND see if the handle is our target.
bool CheckHandle(HANDLE hFile)
{
   if (!IsInitializedAndConnected())
      return false;

   std::unique_lock<std::mutex> lock(GetGlobals()->handleMutex);
   // see if our handle is in the list
   bool returnVal = false;
   if (GetGlobals()->handleVec.empty())
   {
      // Test to see if this is a named pipe & has the target name
      if (CheckNamedPipeName(hFile, GetGlobals()->targetName))
      {
         GetGlobals()->handleVec.push_back(hFile);
         returnVal = true;
      }
   }
   else
   {
      auto foundItem = std::find(std::begin(GetGlobals()->handleVec),
                                 std::end(GetGlobals()->handleVec), hFile);
      if (foundItem != std::end(GetGlobals()->handleVec))
      {
         returnVal = true;
      }
   }
   return returnVal;
}

extern "C" {
__declspec(dllexport) BOOL WINAPI
    m_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
               LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{

	if (CheckHandle(hFile))
	{
		if (lpOverlapped != NULL)
		{
			std::unique_lock<std::mutex> lock(GetGlobals()->waitingMutex);
			GetGlobals()->waitingData.push_back(std::make_pair(lpOverlapped, lpBuffer));
		}
		else
		{
			auto ptr = std::unique_ptr<char[]>(new char[nNumberOfBytesToRead]);
			memcpy_s(ptr.get(), nNumberOfBytesToRead, lpBuffer, nNumberOfBytesToRead);
			// send the pointer and the size into the buffer
			StreamerTools::Buffer mybuf(std::move(ptr), nNumberOfBytesToRead,
				std::chrono::high_resolution_clock::now(),
				StreamerTools::Action::READ);
			GetGlobals()->streamer->pushData(std::move(mybuf));
		}
	}

   return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                   lpOverlapped);
}

__declspec(dllexport) BOOL WINAPI
    m_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
   // Send lpBuffer and nNumberOfBytesToWrite to be pcapped formatted
   if (IsInitializedAndConnected() && CheckHandle(hFile))
   {
      auto ptr = std::unique_ptr<char[]>(new char[nNumberOfBytesToWrite]);
      memcpy_s(ptr.get(), nNumberOfBytesToWrite, lpBuffer, nNumberOfBytesToWrite);
      // send the pointer and the size into the buffer
      StreamerTools::Buffer mybuf(std::move(ptr), nNumberOfBytesToWrite,
                                  std::chrono::high_resolution_clock::now(),
                                  StreamerTools::Action::WRITE);
      GetGlobals()->streamer->pushData(std::move(mybuf));
   }
   return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten,
                    lpOverlapped);
}

__declspec(dllexport) BOOL WINAPI
    m_GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes,
                                PULONG_PTR lpCompletionKey,
                                LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds)
{
   BOOL ret =
       GetQueuedCompletionStatus(CompletionPort, lpNumberOfBytes, lpCompletionKey,
                                 lpOverlapped, dwMilliseconds);

   if (*lpOverlapped == nullptr)
      return ret;

   if (ret == 0)
   {
      // Process error case. Maybe the handle was closed.
      return ret;
   }

   std::pair<LPOVERLAPPED, LPVOID> procData;

   if (!IsInitializedAndConnected())
      return ret;

   {
      std::lock_guard<std::mutex> lg(GetGlobals()->waitingMutex);
      auto currentData = std::find_if(
          GetGlobals()->waitingData.begin(), GetGlobals()->waitingData.end(),
          [&lpOverlapped](const std::pair<LPOVERLAPPED, LPVOID>& element) -> bool
          {
             return element.first == *lpOverlapped;
          });

      if (currentData == GetGlobals()->waitingData.end())
      {
         return ret;
      }
      procData = std::move(*currentData);
      GetGlobals()->waitingData.erase(currentData);
   }

   auto ptr = std::unique_ptr<char[]>(new char[*lpNumberOfBytes]);
   memcpy_s(ptr.get(), *lpNumberOfBytes, procData.second, *lpNumberOfBytes);
   // send the pointer and the size into the buffer
   StreamerTools::Buffer mybuf(std::move(ptr), *lpNumberOfBytes,
                               std::chrono::high_resolution_clock::now(),
                               StreamerTools::Action::READ);
   GetGlobals()->streamer->pushData(std::move(mybuf));

   return ret;
}

__declspec(dllexport) DWORD WINAPI InitializeCaptureMethods(LPVOID inputstruct)
{
   InputStruct* is = static_cast<InputStruct*>(inputstruct);
   std::unique_ptr<globalStruct> tempGlobalObject =
       std::unique_ptr<globalStruct>(new globalStruct);

   tempGlobalObject->targetName = is->target;
   tempGlobalObject->outputName = is->output;
   tempGlobalObject->clientPort = is->clientPort;
   tempGlobalObject->serverPort = is->serverPort;
   tempGlobalObject->service = std::make_shared<::boost::asio::io_service>();
   tempGlobalObject->work_guard = std::make_shared<::boost::asio::io_service::work>(
       ::boost::ref(*tempGlobalObject->service));
   tempGlobalObject->streamer = std::make_shared<StreamerTools::Streamer>();
   tempGlobalObject->starttime = std::chrono::high_resolution_clock::now();

   if (!globalStruct::InitGlobals(tempGlobalObject.get()))
   {
      return 0;
   }
   tempGlobalObject.release();

   auto glbl = GetGlobals();

   for (size_t numThreads = 0; numThreads < 3; numThreads++)
   {
      glbl->globalThreads.emplace_back([]
                                       {
                                          auto ptr = globalStruct::GetGlobals();
                                          if (!ptr)
                                             return;
                                          ptr->service->run();
                                       });
   }

   StartNamedPipeServer(*glbl);


   // Fire off another thread for receive.

   // CycleFcn:
   // {
   // Initiate receive routine.
   // Tell Streamer to start queuing.
   // Poll streamer for data. Watch for a shutdown signal.
   // If we get data, send it on the connection.
   // If send fails, tell the streamer to stop.
   // Shut down the named pipe and attempt to restart connection on failure.
   return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
   {
      hooked = true;
      DWORD dwError = 0;
      g_pReadFile = new CAPIHook("Kernel32.dll", "ReadFile", (PROC)m_ReadFile);
      g_pWriteFile = new CAPIHook("Kernel32.dll", "WriteFile", (PROC)m_WriteFile);
      g_pGetQueuedCompletionStatus =
          new CAPIHook("Kernel32.dll", "GetQueuedCompletionStatus",
                       (PROC)m_GetQueuedCompletionStatus);
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
         delete g_pReadFile;
         delete g_pWriteFile;
         delete g_pGetQueuedCompletionStatus;
         CleanUpEverything();
      }
      break;
   }
   }
   return TRUE;
}
}