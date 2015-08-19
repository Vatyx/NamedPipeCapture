#pragma once

#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>
#include "boost/asio.hpp"
class NamedPipeServer;
class NamedPipe;
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
   std::chrono::high_resolution_clock::time_point starttime;
   std::shared_ptr<::boost::asio::io_service> service;
   std::shared_ptr<::boost::asio::io_service::work> work_guard;
   std::shared_ptr<NamedPipeServer> server;
   std::shared_ptr<NamedPipe> pipe;
   std::shared_ptr<StreamerTools::Streamer> streamer;
   static std::atomic<globalStruct*> globalObject;
   static globalStruct* GetGlobals();
   static bool InitGlobals(globalStruct* obj);
   ~globalStruct();
};
