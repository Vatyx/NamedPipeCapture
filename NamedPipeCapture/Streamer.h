#ifndef STREAMER_HEADER_H
#define STREAMER_HEADER_H

#include "HeaderTypes.h"
#include <iostream>
#include <vector>
#include <chrono>

#include <concurrent_queue.h>
#include <atomic>
#include <thread>
#include <mutex>

#include "SeqAck.h"

class Writer;

namespace StreamerTools
{
	class Buffer;

	class Streamer
	{
	public:
		Streamer()
			: dataQueue()
			, m_ThreadWaitingSignal(false)
         , m_runningFlag(false)
		{
		}

		void pushData(Buffer&&);

		void startStream(Writer&);
		void stream(Writer&);
		void stopStream();

	private:
		bool try_popData(Buffer&);
		void streamBuffer(Writer& writer, Buffer& buff);
		void IncrementRead(std::size_t);
		void IncrementWrite(std::size_t);

		concurrency::concurrent_queue<std::shared_ptr<Buffer>> dataQueue;
		std::thread m_thread;
		std::atomic_bool m_ThreadWaitingSignal;
		std::condition_variable m_Signal;
		std::mutex m_mutex;
		std::atomic<bool> m_runningFlag;
		SeqAckController clientController;
		SeqAckController serverController;
	};

	struct Controller
	{
		concurrency::concurrent_queue<std::shared_ptr<Buffer>> dataQueue;
	};
}
#endif