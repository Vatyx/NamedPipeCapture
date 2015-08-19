#include "Streamer.h"
#include "Buffer.h"
#include "Writer.h"
#include <strstream>
#include <algorithm>
#include <memory>

namespace StreamerTools
{
using namespace HeaderTypes;

void Streamer::pushData(Buffer&& data)
{
	if (!m_runningFlag)
		return;
   auto pbuff = std::make_shared<Buffer>(std::move(data));
   dataQueue.push(pbuff);
   if (m_ThreadWaitingSignal.load())
   {
      std::lock_guard<std::mutex> lg(m_mutex);
      m_ThreadWaitingSignal = false;
      m_Signal.notify_one();
   }
}

void Streamer::startStream(Writer& writer)
{
   bool expected = false;
   if (!m_runningFlag.compare_exchange_strong(expected, true))
      return;
   PcapGlobalHeader pgh;
   std::ostrstream out;
   out << pgh;
   DoWrite(writer, std::make_pair(std::unique_ptr<char[]>(out.str()),
                                  static_cast<std::size_t>(out.pcount())));
}

void Streamer::stopStream()
{
   std::unique_lock<std::mutex> lk(m_mutex);
   m_runningFlag = false;
   m_Signal.notify_all();
   lk.unlock();

   // now let's get all of the data from the queue.
   Buffer current;
   while (try_popData(current)) {}
}

bool Streamer::try_popData(Buffer& buff)
{
   std::shared_ptr<Buffer> pbuff;

   if (dataQueue.try_pop(pbuff))
   {
      buff = std::move(*pbuff);
      return true;
   }
   return false;
}

void Streamer::stream(Writer& writer)
{
   Buffer current;
   do
   {
      while (!try_popData(current) && m_runningFlag.load())
      {
         std::unique_lock<std::mutex> lk(m_mutex);
         m_ThreadWaitingSignal = true;
         m_Signal.wait(lk, [this]
                       {
                          return !m_ThreadWaitingSignal || !m_runningFlag.load();
                       });
      }
      if (!m_runningFlag.load())
         return;
      streamBuffer(writer, std::move(current));

   } while (current.getAction() != Action::END);
}

void Streamer::streamBuffer(Writer& writer, Buffer& buff)
{
   if (buff.getSize() > 0)
   {
      auto buffSplit = buff.split(PACKET_SIZE);
      std::for_each(
          std::begin(buffSplit), std::end(buffSplit),
          [&](decltype(*std::begin(buffSplit))& buf)
          {
             FullDataPacket fdp(std::move(buf), buff.getTime());
             switch (buff.getAction())
             {
             case Action::READ:
                IncrementRead(buf.second);
                fdp.setIPs(SERVER_IP, CLIENT_IP);
                fdp.setSeqAck(serverController.getSeq(), serverController.getAck());
                break;
             case Action::WRITE:
                IncrementWrite(buf.second);
                fdp.setIPs(CLIENT_IP, SERVER_IP);
                fdp.setSeqAck(clientController.getSeq(), clientController.getAck());
                break;
             default:
                // Throw out?
                break;
             }
             std::ostrstream ostr;
             ostr << fdp;
             DoWrite(writer, std::make_pair(std::unique_ptr<char[]>(ostr.str()),
                                            ostr.pcount()));
          });
   }
}

void Streamer::IncrementRead(std::size_t size)
{
   serverController.sendingPacket(size);
   clientController.receivingPacket(size);
}

void Streamer::IncrementWrite(std::size_t size)
{
   clientController.sendingPacket(size);
   serverController.receivingPacket(size);
}

void SeqAckController::sendingPacket(size_t packetSize)
{
   seq += lastPacketSent;
   lastPacketSent = packetSize;
}

void SeqAckController::receivingPacket(size_t packetSize)
{
   lastPacketReceived = packetSize;
   ack += lastPacketReceived;
}
}