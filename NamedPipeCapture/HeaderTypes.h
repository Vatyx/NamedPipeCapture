#ifndef HEADERTYPES_H
#define HEADERTYPES_H

#include <stdint.h>
#include <iostream>
#include <chrono>
#include <memory>

namespace HeaderTypes
{
	const int PCAPGLOBAL_SIZE = 24;
	const int ETHERNETHEADER_SIZE = 14;
	const int IPHEADER_SIZE = 20;
	const int TCPHEADER_SIZE = 20;
	const int TOTALHEADER_SIZE = 70;

	const int PACKET_SIZE = 1400;

	const long CLIENT_IP = 0x0A000001;
	const long SERVER_IP = 0x0A000002;

	struct EthernetHeader
	{
		unsigned long long destAddr;
		unsigned long long sourceAddr;
		unsigned short type;

		static const unsigned long DEFAULT_DESTADDR = 0;
		static const unsigned long DEFAULT_SOURCEADDR = 0;
		static const unsigned short DEFAULT_TYPE = 0x0800;

		EthernetHeader()
		{
			destAddr = DEFAULT_DESTADDR;
			sourceAddr = DEFAULT_SOURCEADDR;
			type = DEFAULT_TYPE;
		}

		friend std::ostream& operator<<(std::ostream&, const EthernetHeader&);
	};

	struct IPHeader
	{
		unsigned char version : 4;			//4 bits	Specifies the format of the IP packet header.
		unsigned char IHL : 4;				//4 bits	Specifies the length of the IP packet header in 32 bit words. The minimum value for a valid header is 5.
		unsigned char diffService;			//1 byte	
		unsigned short totalLength;			//2 bytes	Contains the length of the datagram.
		unsigned short id;					//2 bytes	
		unsigned short flags : 3;			//3 bits
		unsigned short fragmentation : 13;	//13 bits	Used to identify the fragments of one datagram from those of another.
		unsigned char ttl;					//1 byte	A timer field used to track the lifetime of the datagram
		unsigned char protocol;				//1 byte	This field specifies the next encapsulated protocol.
		unsigned short checksum;			//2 bytes	A 16 bit one's complement checksum of the IP header and IP options.		
		unsigned int sourceIP;				//4 bytes	IP address of the sender.
		unsigned int destIP;				//4 bytes	IP address of the intended receiver.

		static const unsigned char DEFAULT_VERSION = 4;	//IPv4
		static const unsigned char DEFAULT_IHL = 5;		//Size of header is 5 WORDS
		static const unsigned char DEFAULT_DIFFSERVICE = 0;
		static const unsigned short DEFAULT_TOTALLENGTH = IPHEADER_SIZE + TCPHEADER_SIZE;
		static const unsigned short DEFAULT_ID = 0;
		static const unsigned char DEFAULT_FLAGS = 0;
		static const unsigned short DEFAULT_FRAGMENTATION = 0;
		static const unsigned char DEFAULT_TTL = 255;
		static const unsigned char DEFAULT_PROTOCOL = 6;
		static const unsigned short DEFAULT_CHECKSUM = 0;
		static const unsigned int DEFAULT_SOURCEIP = 0x000000;
		static const unsigned int DEFAULT_DESTIP = 0x000000;
		
		IPHeader()
		{
			version = DEFAULT_VERSION;
			IHL = DEFAULT_IHL;
			diffService = DEFAULT_DIFFSERVICE;
			totalLength = DEFAULT_TOTALLENGTH;
			id = DEFAULT_ID;
			flags = DEFAULT_FLAGS;
			fragmentation = DEFAULT_FRAGMENTATION;
			ttl = DEFAULT_TTL;
			protocol = DEFAULT_PROTOCOL;
			checksum = DEFAULT_CHECKSUM;
			sourceIP = DEFAULT_SOURCEIP;
			destIP = DEFAULT_DESTIP;
		}

		void setSize(unsigned int dataSize) { totalLength += dataSize; }

		friend std::ostream& operator<<(std::ostream&, const IPHeader&);
	};

	struct TCPHeader
	{
		unsigned short srcPort;			//2 bytes
		unsigned short destPort;		//2 bytes
		unsigned int seqNum;			//4 bytes	The sequence number of the first data byte in this segment.
		unsigned int ackNum;			//4 bytes	If the ACK bit is set, this field contains the value of the next sequence number the sender of the segment is expecting to receive
		unsigned char dataOffset : 4;	//4 bits	The number of 32-bit words in the TCP header. This indicates where the data begins
		unsigned char reserved : 6;		//6 bits 
		unsigned char controlBit : 6;	//6 bits
		unsigned short window;			//2 bytes	The number of bytes beginning with the one indicated in the ack num which sender is willing to accept
		unsigned short checksum;		//2 bytes
		unsigned short urgentPtr;		//2 bytes

		static const unsigned short DEFAULT_SRCPORT = 0;
		static const unsigned short DEFAULT_DESTPORT = 0;
		static const unsigned int DEFAULT_SEQNUM = 0;
		static const unsigned int DEFAULT_ACKNUM = 0;
		static const unsigned char DEFAULT_DATAOFFSET = 5;
		static const unsigned char DEFAULT_RESERVED = 0;
		static const unsigned char DEFAULT_CONTROLBIT = 16;
		static const unsigned short DEFAULT_WINDOW = 2048;
		static const unsigned short DEFAULT_CHECKSUM = 0;
		static const unsigned short DEFAULT_URGENTPTR = 0;

		TCPHeader()
		{
			srcPort = DEFAULT_SRCPORT;
			destPort = DEFAULT_DESTPORT;
			seqNum = DEFAULT_SEQNUM;
			ackNum = DEFAULT_ACKNUM;
			dataOffset = DEFAULT_DATAOFFSET;
			reserved = DEFAULT_RESERVED;
			controlBit = DEFAULT_CONTROLBIT;
			window = DEFAULT_WINDOW;
			checksum = DEFAULT_CHECKSUM;
			urgentPtr = DEFAULT_URGENTPTR;
		}

		friend std::ostream& operator<<(std::ostream&, const TCPHeader&);
	};

	struct PcapGlobalHeader
	{
		uint32_t magic_number;   /* magic number */
		uint16_t version_major;  /* major version number */
		uint16_t version_minor;  /* minor version number */
		int32_t  thiszone;       /* GMT to local correction */
		uint32_t sigfigs;        /* accuracy of timestamps */
		uint32_t snaplen;        /* max length of captured packets, in octets */
		uint32_t network;        /* data link type */

		static const uint32_t DEFAULT_MAGIC_NUMBER = 0xa1b2c3d4;
		static const uint16_t DEFAULT_VERSION_MAJOR = 2;
		static const uint16_t DEFAULT_VERSION_MINOR = 4;
		static const int32_t DEFAULT_GMT_ZONE = 0;
		static const uint32_t DEFAULT_SIGFIGS = 0;
		static const uint32_t DEFAULT_SNAPLEN = 65535;
		static const uint32_t DEFAULT_NETWORK = 1;

		PcapGlobalHeader()
		{
			magic_number = DEFAULT_MAGIC_NUMBER;
			version_major = DEFAULT_VERSION_MAJOR;
			version_minor = DEFAULT_VERSION_MINOR;
			thiszone = DEFAULT_GMT_ZONE;
			sigfigs = DEFAULT_SIGFIGS;
			snaplen = DEFAULT_SNAPLEN;
			network = DEFAULT_NETWORK;
		}

		friend std::ostream& operator<<(std::ostream&, const PcapGlobalHeader&);
	};

	struct PcapPacketHeader
	{
		uint32_t ts_sec;         /* timestamp seconds */
		uint32_t ts_usec;        /* timestamp microseconds */
		uint32_t incl_len;       /* number of octets of packet saved in file */
		uint32_t orig_len;       /* actual length of packet */

		static const uint32_t DEFAULT_TIME_SECONDS = 0;
		static const uint32_t DEFAULT_TIME_MICROSECONDS = 0;
		static const uint32_t DEFAULT_INCL_LEN = ETHERNETHEADER_SIZE + IPHEADER_SIZE + TCPHEADER_SIZE;
		static const uint32_t DEFAULT_ORIG_LEN = ETHERNETHEADER_SIZE + IPHEADER_SIZE + TCPHEADER_SIZE;

		PcapPacketHeader()
		{
			ts_sec = DEFAULT_TIME_SECONDS;
			ts_usec = DEFAULT_TIME_MICROSECONDS;
			incl_len = DEFAULT_INCL_LEN;
			orig_len = DEFAULT_ORIG_LEN;
		}

		void setSize(unsigned int);
		void setTime(std::chrono::high_resolution_clock::time_point);

		friend std::ostream& operator<<(std::ostream&, const PcapPacketHeader&);
	};

	struct FullDataPacket
	{
		PcapPacketHeader pph;
		EthernetHeader eth;
		IPHeader ip;
		TCPHeader tcp;

		std::unique_ptr<char[]> dataLocation;
		size_t dataSize;

		FullDataPacket(): dataLocation(nullptr), dataSize(0) {}
		FullDataPacket(std::unique_ptr<char[]> data, size_t size, std::chrono::high_resolution_clock::time_point duration)
			: dataLocation(std::move(data)), dataSize(size)
		{
			setSize(static_cast<unsigned int>(size));
			setTime(duration);
		}
      FullDataPacket(std::pair<std::unique_ptr<char []>, size_t> datapair, std::chrono::high_resolution_clock::time_point duration)
         : dataLocation(std::move(datapair.first)), dataSize(datapair.second)
      {
         setSize(static_cast<unsigned int>(dataSize));
         setTime(duration);
      }

		void setSize(unsigned int);
		void setTime(std::chrono::high_resolution_clock::time_point);
		void setIPs(unsigned int, unsigned int);
		void setPorts(unsigned short, unsigned short);
		void setSeqAck(unsigned int, unsigned int);

		friend std::ostream& operator<<(std::ostream&, const FullDataPacket&);	
	};
}

#endif