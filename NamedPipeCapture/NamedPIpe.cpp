#include "NamedPipe.h"
#include <utility>
#include "SmartHandle.h"

std::shared_ptr<NamedPipe> NamedPipe::Create(BoostNamedPipeHandle&& pipe)
{
   return std::make_shared<NamedPipe>(std::move(pipe));
}

NamedPipe::~NamedPipe() { Cleanup(); }

NamedPipe::NamedPipe(BoostNamedPipeHandle&& pipe)
   : m_isConnected(true)
   , m_sendErrorCB()
   , m_handle(std::move(pipe))
   , m_sendsInProgress(0)
   , m_sendThreadActive(false)
   , m_cleanupActive(false)
{
}

bool NamedPipe::IsConnected() const { return m_isConnected; }

void NamedPipe::Cleanup()
{
   bool expected = false;
   if (!m_cleanupActive.compare_exchange_strong(expected, true))
      return;
   ServerCleanup();
}

void NamedPipe::ServerCleanup()
{
   if (m_handle.is_open())
   {
      ::DisconnectNamedPipe(m_handle.native_handle());
      boost::system::error_code ec(boost::asio::error::operation_aborted,
                                   boost::asio::error::get_system_category());
      m_handle.close(ec);
   }
   m_isConnected = false;
}

void NamedPipe::SendStream(std::pair<std::unique_ptr<char[]>, size_t> strmToSend)
{
   if (!IsConnected())
      return;
   boost::system::error_code ec;
   ::boost::asio::write(
       m_handle, ::boost::asio::buffer(strmToSend.first.get(), strmToSend.second), ec);

   if (ec && (ec != ::boost::asio::error::operation_aborted))
   {
      // disconnect handler
   }
}

NamedPipe::BoostNamedPipeHandle& NamedPipe::GetHandle() { return m_handle; }
