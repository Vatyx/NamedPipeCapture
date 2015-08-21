#include "CaptureEngine.h"
#include "Globals.h"
#include "NamedPipeServer.h"
#include "NamedPipe.h"
#include "boost/asio/error.hpp"
#include "Writer.h"
#include "Streamer.h"
#include "FcnTracker.hxx"


void StartNamedPipeServer(globalStruct& globalObj)
{
   auto svr = globalObj.GetServer();

   if (!svr)
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
      std::shared_ptr<NamedPipeServer> cmpVal;
      if (std::atomic_compare_exchange_strong(&globalObj.server, &cmpVal, server))
      {
         server->SetNamedPipeSecurity();
      }
      svr = globalObj.GetServer();
   }

   auto CnxnSuccess = [](std::shared_ptr<NamedPipe> pipe)
   {
      if (!pipe || !pipe->IsConnected())
         return;
      ::InitCycle(pipe);
   };

   svr->Start(CnxnSuccess, true);
}

void DoWrite(std::shared_ptr<NamedPipe>& pipe,
             std::pair<std::unique_ptr<char[]>, size_t> thingToWrite)
{
   if (!pipe || !pipe->IsConnected())
      return;
   pipe->SendStream(std::move(thingToWrite));
}

void StartRecv(const std::shared_ptr<NamedPipe>& pipe)
{
   char* SomeBuffer = new char[1024];
   std::weak_ptr<NamedPipe> wptr = pipe;
   ::boost::asio::async_read(
       pipe->GetHandle(), boost::asio::buffer(SomeBuffer, 1024),
       [SomeBuffer, wptr](const boost::system::error_code& ec, std::size_t) mutable
       {
          delete [] SomeBuffer;
          auto sptr = wptr.lock();
          if (!sptr)
             return;
          auto globals = globalStruct::GetGlobals();
          if (!globals)
             return;
          if (ec && (ec != boost::asio::error::operation_aborted)) // We'll be
                                                                   // responsible for
                                                                   // this.
          {
             auto strm = globals->GetStreamer();
             if (strm)
                strm->stopStream();
             // restart the server

             auto pipe = std::atomic_load(&globals->pipe);
             std::shared_ptr<NamedPipe> nullPipe;
             std::atomic_store(&globals->pipe, nullPipe);
             sptr->Cleanup();
             auto svr = globals->GetServer();
             if (!svr)
                return;
             svr->Stop();
             globals->ClearReadCollection();
             StartNamedPipeServer(*globals);
          }
       });
}

void InitCycle(std::shared_ptr<NamedPipe>& pipe)
{
   auto globals = globalStruct::GetGlobals();
   if (!globals)
      return;
   auto svc = globals->GetService();
   if (!svc)
      return;
   std::atomic_store(&globals->pipe, pipe);
   StartRecv(pipe);

   svc->post([pipe, globals]()
                          {
                             if (!pipe || !pipe->IsConnected())
                                return;
                             auto strm = globals->GetStreamer();
                             if (!strm)
                                return;
                             try
                             {
                                Writer MyWriter(pipe);
                                strm->startStream(MyWriter);
                                strm->stream(MyWriter);
                             }
                             catch (NamedPipeException& ex)
                             {
                                pipe->Cleanup();
                             }
                          });
}

void CleanUpEverything()
{
   auto glbl = globalStruct::GetGlobals();
   if (!glbl)
      return;

   auto svr = glbl->GetServer();
   if (svr)
   {
      svr->Stop();
   }

   auto strm = glbl->GetStreamer();
   if (strm)
   {
      strm->stopStream();
   }
   auto pipe = glbl->GetPipe();
   if (pipe)
   {
      pipe->Cleanup();
   }
   glbl->work_guard.reset();
   auto svc = glbl->GetService();

   glbl->ResetAllPointers();

   svr.reset();
   strm.reset();
   pipe.reset();

   if (svc && !svc->stopped())
   {
      svc->stop();
   }
   std::for_each(glbl->globalThreads.begin(), glbl->globalThreads.end(),
                 [](decltype(*glbl->globalThreads.begin())& thr)
                 {
                    thr.join();
                 });
   svc.reset();
   auto trkr = glbl->GetTracker();
   if (trkr)
   {
      trkr->WaitUntilThreadVecEmpty(std::chrono::minutes(10));
   }
   glbl.reset();
   globalStruct::nullifyGlobals();
}