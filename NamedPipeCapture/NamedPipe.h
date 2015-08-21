#pragma once

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <string>
#include <concurrent_queue.h>
#include <Windows.h>
#include <exception>

class NamedPipeException : public std::exception
{
private:
   NamedPipeException();
public:
   explicit NamedPipeException(const ::boost::system::error_code& val) : iCode(val) {}
   virtual ~NamedPipeException() {}

   NamedPipeException(const NamedPipeException& other)
      : iCode(other.iCode)
   {
   }
   NamedPipeException& operator=(const NamedPipeException& other)
   {
      iCode = other.iCode;
   }
   virtual char const * what() const throw()
   {
      return "Generic Named Pipe Exception";
   }
   ::boost::system::error_code GetCode() const { return iCode; }
protected:
   ::boost::system::error_code iCode;
};

class NamedPipe : public std::enable_shared_from_this<NamedPipe>
{
   struct HiddenType;
public:
	typedef ::boost::asio::windows::stream_handle BoostNamedPipeHandle;

	NamedPipe(const HiddenType&, BoostNamedPipeHandle&&);

	static std::shared_ptr<NamedPipe>
		Create(BoostNamedPipeHandle&&);
	
	~NamedPipe();

	bool IsConnected() const;
	void Cleanup();

	void SendStream(std::pair<std::unique_ptr<char[]>, size_t> strmToSend);
   BoostNamedPipeHandle& GetHandle();

private:
	void ServerCleanup();

private:
	bool m_isConnected;
	bool m_isEstablished; // Indicates that the protocol exchanges have completed.
	std::string m_endpoint;

   BoostNamedPipeHandle m_handle;

	std::mutex m_mutex;

	std::atomic<unsigned int> m_sendsInProgress;
	std::atomic<bool> m_sendThreadActive;
	std::atomic_flag m_cleanupActive;
	std::function<void(void)> m_cleanupFcn;
	
};