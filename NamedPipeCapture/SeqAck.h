#pragma once
namespace StreamerTools
{
   class SeqAckController
   {
   public:
      SeqAckController() : seq(0), ack(0), lastPacketSent(0), lastPacketReceived(0) {}
      SeqAckController(unsigned int s, unsigned int a) : SeqAckController()
      {
         seq = s;
         ack = a;
      }

      void sendingPacket(size_t);
      void receivingPacket(size_t);

      unsigned int getSeq() const { return seq; }
      unsigned int getAck() const { return ack; }

   private:
      unsigned int seq;
      unsigned int ack;
      size_t lastPacketSent;
      size_t lastPacketReceived;
   };
}