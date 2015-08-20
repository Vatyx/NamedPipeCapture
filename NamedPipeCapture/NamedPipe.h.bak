#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <string>
#include <concurrent_queue.h>
#include <Windows.h>

class NamedPipe 
{

public:
	typedef std::function<void(const ::boost::system::error_code&)> ErrorHandler;
	typedef ::boost::asio::windows::stream_handle BoostNamedPipeHandle;
	//typedef std::function<void(uRStream)> ReceivedStreamHandlerProc;
	//typedef std::function<void(uRStream, std::function<void()>)> ReceivedStreamHandler;

	NamedPipe(BoostNamedPipeHandle&&);

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

	ErrorHandler m_sendErrorCB;
	::boost::asio::windows::stream_handle m_handle;

	std::mutex m_mutex;

	std::atomic<unsigned int> m_sendsInProgress;
	std::atomic<bool> m_sendThreadActive;
	std::atomic<bool> m_cleanupActive;
	std::function<void(void)> m_cleanupFcn;
	
};