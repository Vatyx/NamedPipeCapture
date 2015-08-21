#include "Globals.h"

#include <atomic>
#include "NamedPipe.h"
#include "NamedPipeServer.h"
#include "FcnTracker.hxx"

std::shared_ptr<globalStruct> globalStruct::globalObject = nullptr;

std::shared_ptr<globalStruct> globalStruct::GetGlobals()
{
   return std::atomic_load(&globalObject);
}

bool globalStruct::InitGlobals(const std::shared_ptr<globalStruct>& obj)
{
   std::shared_ptr<globalStruct> pVal;
   return std::atomic_compare_exchange_strong(&globalObject, &pVal, obj);
}

std::shared_ptr<globalStruct> globalStruct::nullifyGlobals()
{
   auto retval = GetGlobals();
   if (!retval)
      return retval;
   std::shared_ptr<globalStruct> resetVal;
   std::atomic_store(&globalObject, resetVal);
   return retval;
}

void globalStruct::ClearReadCollection()
{
   std::lock_guard<std::mutex> lg(waitingMutex);
   waitingData.clear();
}

bool globalStruct::FindAndEraseReadData(LPOVERLAPPED lpOverlapped, std::pair<LPOVERLAPPED, LPVOID>& procData)
{
   std::lock_guard<std::mutex> lg(waitingMutex);
   auto currentData = std::find_if(
      GetGlobals()->waitingData.begin(), GetGlobals()->waitingData.end(),
      [&lpOverlapped](const std::pair<LPOVERLAPPED, LPVOID>& element) -> bool
   {
      return element.first == lpOverlapped;
   });

   if (currentData == GetGlobals()->waitingData.end())
   {
      return false;
   }
   procData = std::move(*currentData);
   waitingData.erase(currentData);
   return true;
}

std::shared_ptr<StreamerTools::Streamer> globalStruct::GetStreamer()
{
   return std::atomic_load(&streamer);
}

std::shared_ptr<boost::asio::io_service> globalStruct::GetService()
{
   return std::atomic_load(&service);
}

std::shared_ptr<NamedPipeServer> globalStruct::GetServer()
{
   return std::atomic_load(&server);
}

std::shared_ptr<NamedPipe> globalStruct::GetPipe()
{
   return std::atomic_load(&pipe);
}

std::shared_ptr<FunctionUsageTracker> globalStruct::GetTracker()
{
   return std::atomic_load(&fcnTracker);
}

void globalStruct::ResetAllPointers()
{
   std::shared_ptr<StreamerTools::Streamer> resetStream;
   std::atomic_store(&streamer, resetStream);
   std::shared_ptr<NamedPipe> resetPipe;
   std::atomic_store(&pipe, resetPipe);
   std::shared_ptr<NamedPipeServer> resetServer;
   std::atomic_store(&server, resetServer);
   std::shared_ptr<boost::asio::io_service> resetSvc;
   std::atomic_store(&service, resetSvc);
}

globalStruct::~globalStruct() {}
