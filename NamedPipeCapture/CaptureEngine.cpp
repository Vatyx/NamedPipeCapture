#include "CaptureEngine.h"
#include "Globals.h"
#include "NamedPipeServer.h"
#include "NamedPipe.h"
#include "boost/asio/error.hpp"
#include "Writer.h"


void StartNamedPipeServer(globalStruct& globalObj)
{
   if (!globalObj.server)
   {
      auto serverErrFcn = [](const ::boost::system::error_code& ec)
      {
         if (ec != boost::asio::error::operation_aborted)
         {
            auto MyObj = globalStruct::GetGlobals();
            if (!MyObj)
               return;
            StartNamedPipeServer(*MyObj);
         }
      };

      auto server = NamedPipeServer::Create(globalObj.service, globalObj.outputName,
                                            serverErrFcn);

      std::atomic_store(&globalObj.server, server);
      globalObj.server->SetNamedPipeSecurity();
   }

   auto CnxnSuccess = [&](std::shared_ptr<NamedPipe> pipe)
   {
      ::InitCycle(pipe);
   };
   globalObj.server->Start(CnxnSuccess, true);
}

void DoWrite(std::shared_ptr<NamedPipe>& pipe,
             std::pair<std::unique_ptr<char[]>, size_t> thingToWrite)
{
   if (!pipe->IsConnected())
      return;
   pipe->SendStream(std::move(thingToWrite));
}

void StartRecv(const std::shared_ptr<NamedPipe>& pipe)
{
   char* SomeBuffer = new char[1024];
   ::boost::asio::async_read(
       pipe->GetHandle(), boost::asio::buffer(SomeBuffer, 1024),
       [SomeBuffer](const boost::system::error_code& ec, std::size_t) mutable
       {
          delete[] SomeBuffer;
          auto globals = globalStruct::GetGlobals();
          if (!globals)
             return;
          if (ec && (ec != boost::asio::error::operation_aborted)) // We'll be
                                                                   // responsible for
                                                                   // this.
          {
             globals->streamer->stopStream();
             // restart the server
             auto pipe = std::atomic_load(&globals->pipe);
             std::shared_ptr<NamedPipe> nullPipe;
             std::atomic_store(&globals->pipe, nullPipe);
             pipe->Cleanup();
             globals->server->Stop();
             StartNamedPipeServer(*globals);
          }
       });
}

void InitCycle(std::shared_ptr<NamedPipe>& pipe)
{
   auto globals = globalStruct::GetGlobals();
   if (!globals)
      return;
   std::atomic_store(&globals->pipe, pipe);
   StartRecv(pipe);

   globals->service->post([pipe, globals]()
                          {
                             Writer MyWriter(pipe);
                             globals->streamer->startStream(MyWriter);
                             globals->streamer->stream(MyWriter);
                          });
}

void CleanUpEverything()
{
   auto glbl = globalStruct::GetGlobals();
   if (!glbl)
      return;
   glbl->streamer->stopStream();
   glbl->pipe->Cleanup();
   glbl->pipe.reset();
   if (glbl->server)
      glbl->server->Stop();
   glbl->server.reset();
   glbl->work_guard.reset();
   std::for_each(glbl->globalThreads.begin(), glbl->globalThreads.end(),
                 [](decltype(*glbl->globalThreads.begin())& thr)
                 {
                    thr.join();
                 });
   glbl->service.reset();
}