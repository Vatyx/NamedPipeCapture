#pragma once

#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>
#include "boost/asio.hpp"

class NamedPipeServer;
class NamedPipe;
class FunctionUsageTracker;

namespace StreamerTools
{
class Streamer;
}

struct globalStruct
{
   std::mutex handleMutex;
   std::vector<HANDLE> handleVec;
   std::vector<std::pair<LPOVERLAPPED, LPVOID>> waitingData;
   std::mutex waitingMutex;
   std::vector<std::thread> globalThreads;
   std::string targetName;
   std::string outputName;
   unsigned short clientPort;
   unsigned short serverPort;
   std::chrono::high_resolution_clock::time_point starttime;
   std::shared_ptr<::boost::asio::io_service> service;
   std::shared_ptr<::boost::asio::io_service::work> work_guard;
   std::shared_ptr<NamedPipeServer> server;
   std::shared_ptr<NamedPipe> pipe;
   std::shared_ptr<StreamerTools::Streamer> streamer;
   std::shared_ptr<FunctionUsageTracker> fcnTracker;

   static std::shared_ptr<globalStruct> globalObject;
   static std::shared_ptr<globalStruct> GetGlobals();
   static bool InitGlobals(const std::shared_ptr<globalStruct>& obj);
   static std::shared_ptr<globalStruct> nullifyGlobals();
   void ClearReadCollection();
   bool FindAndEraseReadData(LPOVERLAPPED pOvr, std::pair<LPOVERLAPPED, LPVOID>& procData);
   std::shared_ptr<StreamerTools::Streamer> GetStreamer();
   std::shared_ptr<boost::asio::io_service> GetService();
   std::shared_ptr<NamedPipeServer> GetServer();
   std::shared_ptr<NamedPipe> GetPipe();
   std::shared_ptr<FunctionUsageTracker> GetTracker();

   void ResetAllPointers();
   ~globalStruct();
};
