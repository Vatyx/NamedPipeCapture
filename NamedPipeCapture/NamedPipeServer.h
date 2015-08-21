#pragma once
#ifndef NAMEDPIPESERVER_H
#define NAMEDPIPESERVER_H

#include <boost/asio.hpp>
#include "boost/asio/error.hpp"
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <string>
#include <Windows.h>
#include <utility>
#include "NamedPipe.h"
#include "SmartHandle.h"

class NamedPipe;

class NPsecurityattributes
{
   std::shared_ptr<SECURITY_DESCRIPTOR> m_descriptor;
   SECURITY_ATTRIBUTES m_secattr;
   std::shared_ptr<ACL> m_acl;

public:
   explicit NPsecurityattributes(
       std::shared_ptr<SECURITY_DESCRIPTOR> pDescriptor = nullptr,
       BOOL bInherit = FALSE, std::shared_ptr<ACL> pDACL = nullptr);

   NPsecurityattributes(NPsecurityattributes& other);
   NPsecurityattributes& operator=(const NPsecurityattributes& other);

   NPsecurityattributes(NPsecurityattributes&& other);
   NPsecurityattributes& operator=(NPsecurityattributes&& other);

   operator LPSECURITY_ATTRIBUTES();
   LPSECURITY_ATTRIBUTES operator&();
   ~NPsecurityattributes();
};

class NamedPipeServer : public std::enable_shared_from_this<NamedPipeServer>
{
   struct HiddenStruct;
public:
   typedef std::function<void(std::shared_ptr<NamedPipe>)> ConnectionInitFcn;
   typedef std::function<void(const ::boost::system::error_code&)> ErrorHandler;
   typedef std::function<void(const ::boost::system::error_code&)> ASIOCallBack;

   static std::shared_ptr<NamedPipeServer>
   Create(std::shared_ptr<boost::asio::io_service> io, std::string pipename,
          ErrorHandler errfcn);
   NamedPipeServer(const HiddenStruct&, std::shared_ptr<boost::asio::io_service> io, std::string pipename,
                   ErrorHandler errfcn,
                   NPsecurityattributes secattr = NPsecurityattributes()),

       ~NamedPipeServer();

   void Start(ConnectionInitFcn CycleFcn, bool FirstTime = true);
   void Stop();
   void ClearFlagOnDisconnect();

   bool IsRunning() const;
   void SetErrorFcn(ErrorHandler fcn);
   bool SetNamedPipeSecurity();

private:
   NamedPipeHandle BuildNamedPipe(bool IsInitial);
   void InitNamedPipeConnect(OVERLAPPED& OverlappedIO, ASIOCallBack fcn);
   void HandleNewConnection(const boost::system::error_code& ec,
                            std::shared_ptr<OVERLAPPED> pOverlapped,
                            ConnectionInitFcn successCBfunc);

private:
   std::shared_ptr<boost::asio::io_service> s_io_service;
   std::string m_pipeName;
   std::shared_ptr<NamedPipe::BoostNamedPipeHandle> m_namedPipeHandle;
   std::shared_ptr<boost::asio::windows::object_handle>
       m_namedPipeConnectionNotification;
   ErrorHandler m_errorFcn;
   std::mutex m_mutex;
   std::atomic_flag m_connectionTest;
   bool m_isRunning;
   NPsecurityattributes m_SA;
   const UINT32 m_bufferSizeBytes;
};


#endif