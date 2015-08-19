#include "HeaderTypes.h"
#include <chrono>
#include <WinSock2.h>
#include <iostream>

namespace HeaderTypes
{
std::ostream& operator<<(std::ostream& os, const EthernetHeader& eth)
{
   // unsigned long long destAddr = htonll(eth.destAddr);
   // os.write((char*)&destAddr, sizeof(destAddr));

   // unsigned long sourceAddr = htonl(eth.sourceAddr);
   // os.write((char*)&sourceAddr, sizeof(sourceAddr));

   unsigned long filler = 0;
   os.write((char*)&filler, sizeof(filler));
   os.write((char*)&filler, sizeof(filler));
   os.write((char*)&filler, sizeof(filler));

   unsigned short type = htons(eth.type);
   os.write((char*)&type, sizeof(type));

   return os;
}

std::ostream& operator<<(std::ostream& os, const IPHeader& ip)
{
   unsigned char versionIHL = (ip.version << 4 | ip.IHL);
   os.write((char*)&versionIHL, sizeof(versionIHL));

   os.write((char*)&ip.diffService, sizeof(ip.diffService));

   unsigned short totalLength = htons(ip.totalLength);
   os.write((char*)&totalLength, sizeof(totalLength));

   unsigned short id = htons(ip.id);
   os.write((char*)&id, sizeof(id));

   unsigned short flagsFrag = (ip.flags << 13 | ip.fragmentation);
   flagsFrag = htons(flagsFrag);
   os.write((char*)&flagsFrag, sizeof(flagsFrag));

   os.write((char*)&ip.ttl, sizeof(ip.ttl));

   os.write((char*)&ip.protocol, sizeof(ip.protocol));

   unsigned short checksum = htons(ip.checksum);
   os.write((char*)&checksum, sizeof(checksum));

   unsigned int srcIP = htonl(ip.sourceIP);
   os.write((char*)&srcIP, sizeof(srcIP));

   unsigned int destIP = htonl(ip.destIP);
   os.write((char*)&destIP, sizeof(destIP));

   return os;
}

void PcapPacketHeader::setSize(unsigned int dataSize)
{
   if (65535 > (incl_len + dataSize))
      incl_len += dataSize;
   else
      incl_len = 65535;
   orig_len += dataSize;
}

std::ostream& operator<<(std::ostream& os, const TCPHeader& tcp)
{
   unsigned short srcPort = htons(tcp.srcPort);
   os.write((char*)&srcPort, sizeof(srcPort));

   unsigned short destPort = htons(tcp.destPort);
   os.write((char*)&destPort, sizeof(destPort));

   unsigned int seqNum = htonl(tcp.seqNum);
   os.write((char*)&seqNum, sizeof(seqNum));

   unsigned int ackNum = htonl(tcp.ackNum);
   os.write((char*)&ackNum, sizeof(ackNum));

   unsigned short offsetResECNControlBit =
       ((tcp.dataOffset << 12) | (tcp.reserved << 6) | tcp.controlBit);
   offsetResECNControlBit = htons(offsetResECNControlBit);
   os.write((char*)&offsetResECNControlBit, sizeof(offsetResECNControlBit));

   unsigned short window = htons(tcp.window);
   os.write((char*)&window, sizeof(window));

   unsigned short checksum = htons(tcp.checksum);
   os.write((char*)&checksum, sizeof(checksum));

   unsigned short urgentPtr = htons(tcp.urgentPtr);
   os.write((char*)&urgentPtr, sizeof(urgentPtr));

   return os;
}

std::ostream& operator<<(std::ostream& os, const PcapGlobalHeader& pgh)
{
   os.write((char*)&pgh.magic_number, sizeof(pgh.magic_number));

   os.write((char*)&pgh.version_major, sizeof(pgh.version_major));

   os.write((char*)&pgh.version_minor, sizeof(pgh.version_minor));

   os.write((char*)&pgh.thiszone, sizeof(pgh.thiszone));

   os.write((char*)&pgh.sigfigs, sizeof(pgh.sigfigs));

   os.write((char*)&pgh.snaplen, sizeof(pgh.snaplen));

   os.write((char*)&pgh.network, sizeof(pgh.network));

   return os;
}

std::ostream& operator<<(std::ostream& os, const PcapPacketHeader& pph)
{
   os.write((char*)&pph.ts_sec, sizeof(pph.ts_sec));

   os.write((char*)&pph.ts_usec, sizeof(pph.ts_usec));

   os.write((char*)&pph.incl_len, sizeof(pph.incl_len));

   os.write((char*)&pph.orig_len, sizeof(pph.orig_len));

   return os;
}

void PcapPacketHeader::setTime(std::chrono::high_resolution_clock::time_point timept)
{
	auto sysTimePt = (std::chrono::high_resolution_clock::now() - timept) + std::chrono::system_clock::now();
	auto duration = sysTimePt.time_since_epoch();
   auto usec =
       std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
   auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
   usec = usec % sec;

   ts_sec = sec;
   ts_usec = usec;
}

std::ostream& operator<<(std::ostream& os, const FullDataPacket& fh)
{
   os << fh.pph;
   os << fh.eth;
   os << fh.ip;
   os << fh.tcp;
   os.write(fh.dataLocation.get(), fh.dataSize);

   return os;
}

void FullDataPacket::setSize(unsigned int dataSize)
{
   pph.setSize(dataSize);
   ip.setSize(dataSize);
}

void FullDataPacket::setTime(std::chrono::high_resolution_clock::time_point duration)
{
   pph.setTime(duration);
}

void FullDataPacket::setIPs(unsigned int src, unsigned int dest)
{
   ip.sourceIP = src;
   ip.destIP = dest;
}

void FullDataPacket::setSeqAck(unsigned int seq, unsigned int ack)
{
   tcp.seqNum = seq;
   tcp.ackNum = ack;
}
}