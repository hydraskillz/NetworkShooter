/*
 * Author      : Matthew Johnson
 * Date        : 20/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	// IP Address type
	struct IPaddress
	{
		uint32 Host;
		uint16 Port;
	};
	
	typedef uint32 packetID_t;
	typedef uint32 clientID_t;

	// Options for out going packets
	enum SendDataOpts
	{
		SENDOPT_NONE					= 0x0000,		// Out-of-order/non-reliable
		SENDOPT_INORDER					= 0x0001,		// In-order/non-reliable
		SENDOPT_RELIABLE				= 0x0002,		// Out-of-order/reliable
		SENDOPT_INORDER_RELIABLE		= SENDOPT_INORDER | SENDOPT_RELIABLE,
		SENDOPT_CONNECT_REQUEST			= 0x0004,
		SENDOPT_CONNECT_ACCEPT			= 0x0008,
		SENDOP_DISCONNECT				= 0x0010,
		SENDOP_TIMESYNC  				= 0x0020,
	};

	// Header appended to out going packets
	struct PacketHeader
	{
		double     Timestamp;			// Time packet was sent (ms)
		packetID_t PacketID;			// Increasing ID number for packet
		uint32     Flags;				// Special flags for SendDataOpts
										// 0 | In-Order flag (1->in order, 0->out of order)
										// 1 | Reliable flag (1->reliable, 0->not reliable)
										// 2 | Connection request
										// 3 | Connection accept
										// 4 | Disconnecting
										// 5 | Timesync - syncs to timestamp
		uint32     PacketAcks;			// Number of PacketID acknowledgments follow the header
	};	// 24b (8b align)
	
	struct GenericSocket
	{
		int Ready;		// 1 if the socket has data ready
	};

	// defined in NetManager.cpp
	typedef struct GenericSocket* genericSocket_t;
	typedef struct udpSocket* udpSocket_t;
	typedef struct tcpSocket* tcpSocket_t;
	typedef struct _SocketArray* SocketArray;
	
	// UDP Packet type
	struct udpPacket
	{
		udpPacket()
			: DataLength( 0 )
			, MaxDataLength( 0 )
			, Status( 0 )
			, Data( 0 )
		{}
		udpPacket( int size )
			: DataLength( 0 )
			, MaxDataLength( 0 )
			, Status( 0 )
			, Data( 0 )
		{
			Resize( size );
		}
		~udpPacket()
		{
			if ( Data )
				delete[] Data;
		}

		void Resize( int newSize )
		{
			uint8* newData = new uint8[ newSize ];
			if ( Data )
				delete[] Data;
			Data = newData;
			MaxDataLength = newSize;
		}

		double Timestamp;			// Time packet was sent (ms)
		uint8* Data;
		int DataLength;
		int MaxDataLength;
		int Status;
		IPaddress Address;
	};

}