/*
 * Author      : Matthew Johnson
 * Date        : 21/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class NetSession
	{
	public:
		typedef void(*ClientConnectCB)( clientID_t clientID, IPaddress clientAddr );

		static clientID_t IdFromAddress( const IPaddress& addr );

		NetSession();
		~NetSession();

		// Packets larger than this size will be rejected. default=512
		void SetMaxPacketSize( int size )				{ mRecvPacket.Resize( size ); }

		void OpenPort( uint16 port );
		void OnUpdate( /*float dt*/ );

		LocalClient& CreateLocalClient();

		// Poll if a packet is ready to be read
		bool PacketIsReady();
		bool PacketIsReady( clientID_t clientID );
		// Read last received data into a PacketReader
		void ReceiveData( PacketReader& reader, NetClient& sender );
		void ReceiveData( PacketReader& reader, clientID_t clientID );
		// Send data to address. data will be cleared out if sent
		void SendData( PacketWriter& data, IPaddress& addr, int opts, bool clearOnSend=true );
		// Send data to all NetClients
		void SendData( PacketWriter& data, int opts );
		// Send a connection request message
		void SendConnectMessage( IPaddress& addr );
		// Send a connection accept message
		void SendAcceptMessage( IPaddress& addr );

		void DropClient( clientID_t clientID );
		void DropAllClients();

		// Called when a new client connects to this session
		void RegisterClientConnectCallback( ClientConnectCB cb ) { mClientConnectCB = cb; }
		void RegisterClientDisconnectCallback( ClientConnectCB cb ) { mClientDisconnectCB = cb; }

		void SetPacketLoss( int packetLoss )				{ mPacketLoss = packetLoss; }
		double GetAverageRTT( clientID_t clientID )			{ return mClientInfos[clientID].AverageRTTSeconds; }
		int GetMaxSentPacketSize() const					{ return mSendPacket.MaxDataLength; }
		int GetMaxRecvPacketSize() const					{ return mLargestPacketRcv; }
		int GetLastSentPacketSize() const					{ return mSendPacket.DataLength; }
		int GetLastRecvPacketSize() const					{ return mRecvPacket.DataLength; }
		int GetTotalPacketsSent() const						{ return mTotalPacketsSent; }
		int GetTotalPacketsRecv() const						{ return mTotalPacketsRecv; }
		double GetNetTimeSeconds() const					{ return mNetClock->GetElapsedTime( Clock::TIME_SEC ); }

		bool VerboseDebugMsg;
	private:
		int mPacketLoss;
		int mLargestPacketRcv;
		int mTotalPacketsSent;
		int mTotalPacketsRecv;

		ClientConnectCB mClientConnectCB;
		ClientConnectCB mClientDisconnectCB;
		std::vector< clientID_t > mNewClients;
		std::vector< clientID_t > mDeadClients;

		Clock* mNetClock;					// Keep track of time for when send/recv packets
		double mReliableResendTimeout;		// How long to wait before resending reliable packets (ms)

		udpSocket_t mSock;
		udpPacket mSendPacket;
		udpPacket mRecvPacket;

		LocalClient mLocalClient;
		//std::vector< NetClient > mNetClients;

		struct AckInfo
		{
			packetID_t PacketID;			// ID of packet needing ack
			udpPacket  Packet;				// The packet that needs ackd
			double     TimeLastSent;		// Time since we last sent this packet (ms)
			uint32	   Flags;				// Header flags this packet was sent with
		};

		struct ClientInfo
		{
			ClientInfo()
				: LastRecvPacketID( 0 )
				, LastSendPacketID( 0 )
				, LastSendTime( 0 )
				, AverageRTTSeconds( 0 )
			{}
			~ClientInfo()
			{
				while ( !PacketQueue.empty() )
				{
					udpPacket* p = PacketQueue.front();
					delete p;
					PacketQueue.pop();
				}
				DestroyVector( PacketsNeedingAck );
			}
			bool IsPacketReady() const { return !PacketQueue.empty(); }
			std::queue< udpPacket* > PacketQueue;							// Packets from this client
			IPaddress Address;												// Clients address
			packetID_t LastRecvPacketID;									// LastID received from this client
			packetID_t LastSendPacketID;									// LastID sent to this client
			double LastSendTime;											// Last time packet sent to this client (ms)
			std::vector< AckInfo* > PacketsNeedingAck;						// Packets sent to this client needing acknowledgment
			std::vector< packetID_t > PacketsToAck;							// Packets this client needs to acknowledge
			std::map< packetID_t, bool > PacketsAcked;
			double AverageRTTSeconds;
		};

		std::map< clientID_t, ClientInfo > mClientInfos;

	};

}