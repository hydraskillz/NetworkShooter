#include "NetLib.h"

using namespace mage;

clientID_t NetSession::IdFromAddress( const IPaddress& addr )
{
	std::stringstream ss;
	ss << addr.Host << ":" << addr.Port;
	clientID_t senderID = GenerateHash( ss.str().c_str() );
	return senderID;
}

//---------------------------------------
NetSession::NetSession()
	: mSock( 0 )
	, mPacketLoss( 0 )
	, mLargestPacketRcv( 0 )
	, VerboseDebugMsg( false )
	, mTotalPacketsSent( 0 )
	, mTotalPacketsRecv( 0 )
	, mReliableResendTimeout( 1000 )		// 1 sec
	, mClientConnectCB( 0 )
	, mClientDisconnectCB( 0 )
{
	mRecvPacket.Resize( 1024 );
	if ( !NetManager::Init() )
	{
		ConsolePrintf( CONSOLE_WARNING, "NetSession : NetManager init failed!\n" );
	}

	mNetClock = Clock::CreateClock( &Clock::Initialize() );
}
//---------------------------------------
NetSession::~NetSession()
{
	NetManager::udpCloseSocket( mSock );
	NetManager::Quit();
	Clock::DestroyClock( mNetClock );
}
//--------------------------------------
void NetSession::OpenPort( uint16 port )
{
	if ( !mSock )
		//NetManager::udpCloseSocket( mSock );
	mSock = NetManager::udpOpenPort( port );
}
//--------------------------------------
void NetSession::OnUpdate( /*float dt*/ )
{
	// don't call this since we are parented to the main clock which is advanced by the app
	//mNetClock->AdvanceTime( dt );

	if ( mSock == 0 )
		return;

	while ( NetManager::udpRecvPacket( mSock, mRecvPacket ) )
	{
		clientID_t senderID = IdFromAddress( mRecvPacket.Address );

		++mTotalPacketsRecv;

		PacketHeader header;
		ClientInfo& info = mClientInfos[ senderID ];
		info.Address = mRecvPacket.Address;

		// @TODO it might be better to check the 'request' and 'accept' flags on the packet header.
	//	if ( info.LastRecvPacketID == 0 )
	//	{
	//		// Mark new clients so we can notify client code of new connections
	//		// I don't call the callback here b/c client code might try to
	//		// modify the session state causing bad stuff to happen.
	//		mNewClients.push_back( senderID );
	//	}

		// Skip packets that are empty
		if ( mRecvPacket.DataLength > 0 )
		{
			if ( mRecvPacket.DataLength > mLargestPacketRcv )
			{
				mLargestPacketRcv = mRecvPacket.DataLength;
			}

			// Read header
			int headerSize = sizeof( PacketHeader );
			memcpy( &header, mRecvPacket.Data, headerSize );

			if ( VerboseDebugMsg )
			{
				ConsolePrintf( C_FG_LIGHT_GREEN, ">>>>> " );
				ConsolePrintf( "Recv packet: id=%u size=%s\n", header.PacketID, ByteDisplay( mRecvPacket.DataLength ).ToString() );
			}

			// Check acknowledgments received
			uint32 numAcks = header.PacketAcks;
			if ( numAcks > 0 && info.PacketsNeedingAck.size() > 0 )
			{
				for ( uint32 i = 0; i < numAcks; ++i )
				{
					// Acknowledge packet
					packetID_t ackID;
					memcpy( &ackID, mRecvPacket.Data + headerSize + i * sizeof( packetID_t ), sizeof( packetID_t ) );

					// Remove packet from list
					info.PacketsNeedingAck.erase( 
						std::remove_if( info.PacketsNeedingAck.begin(), info.PacketsNeedingAck.end(), [&]( AckInfo*& ackInfo ) -> bool
						{
							bool _ret = ackInfo->PacketID == ackID;
							if ( _ret )
							{
								if ( VerboseDebugMsg )
								{
									ConsolePrintf( C_FG_GREEN, ">>>>> " );
									ConsolePrintf( "ACK recv for packet %u from %u\n", ackID, senderID );
								}
								delete ackInfo;
							}
							return _ret;
						}),
						info.PacketsNeedingAck.end() );
				}
			}

			// Add packet to packets needing acknowledged list
			bool firstTimeAck = true;
			if ( IsBitSet( header.Flags, 1 ) )
			{
				info.PacketsToAck.push_back( header.PacketID );

				// Check if we have acknowledged this packet before
				if ( info.PacketsAcked[ header.PacketID ] )
				{
					firstTimeAck = false;
				}
				// Mark the packet as acknowledge so we can ignore others that may arrive late
				info.PacketsAcked[ header.PacketID ] = true;
			}

			// Do not evaluate packet further - it's a duplicate reliable packet
			if ( !firstTimeAck )
			{
				if ( VerboseDebugMsg )
				{
					ConsolePrintf( C_FG_RED, ">>>>> " );
					ConsolePrintf( "Ignoring packet %u (already received)\n", header.PacketID );
				}
				continue;
			}

			// Check order - receive if new
			bool inOrder = IsBitSet( header.Flags, 0 );
			bool packetIsNew = header.PacketID > info.LastRecvPacketID;
			if ( ( inOrder && packetIsNew ) || ( !inOrder ) )
			{
				// Update average RTT
				double now = mNetClock->GetElapsedTime( Clock::TIME_MILLI );
				double diff = now - header.Timestamp;
				info.AverageRTTSeconds = ( 0.9 * info.AverageRTTSeconds ) + ( 0.1 * diff );

				if ( packetIsNew )
					info.LastRecvPacketID = header.PacketID;

				// Packet was requesting connection
				if ( IsBitSet( header.Flags, 2 ) )
				{
					SendAcceptMessage( mRecvPacket.Address );
					mNewClients.push_back( senderID );
				}

				// Packet was response to requesting connection
				if ( IsBitSet( header.Flags, 3 ) )
				{
					ConsolePrintf( C_FG_GREEN, ">>>>> " );
					ConsolePrintf( "Connection Accepted\n" );
					mNewClients.push_back( senderID );
				}

				// Packet was informing disconnect
				if ( IsBitSet( header.Flags, 4 ) )
				{
					ConsolePrintf( C_FG_YELLOW, ">>>>> " );
					ConsolePrintf( "Removing client (disconnected) %u\n", senderID );
					mClientInfos.erase( mClientInfos.find( senderID ) );
					mDeadClients.push_back( senderID );
					continue;
				}

				// Packet is informing timesync
				if ( IsBitSet( header.Flags, 5 ) )
				{
					ConsolePrintf( C_FG_WHITE, ">>>>> " );
					ConsolePrintf( "Syncing client time to server %f\n", header.Timestamp );
					info.AverageRTTSeconds = 0.0;
					mNetClock->SetTime( header.Timestamp / 1000.0 );
				}

				// If there is user data copy it over to the client
				int clientDataSize = mRecvPacket.DataLength - headerSize - numAcks * sizeof( packetID_t );
				if ( clientDataSize > 0 )
				{
					udpPacket* packet = new udpPacket( mRecvPacket.DataLength );

					// Copy user data - stripping header and acks
					packet->DataLength = mRecvPacket.DataLength - headerSize - numAcks * sizeof( packetID_t );
					memcpy( packet->Data, mRecvPacket.Data + headerSize + numAcks * sizeof( packetID_t ), packet->DataLength );
					packet->Timestamp = header.Timestamp;
					info.PacketQueue.push( packet );
				}
			}
			else
			{
				ConsolePrintf( "Ignoring out-of-order packet : %d\n", header.PacketID );
			}
		}
		else
		{
			ConsolePrintf( "Received empty packet... ignoring\n" );
		}
	}


	// @TODO THIS CODE FEELS MESSY AND INEFFCIENT SHOULD REFACTOR WHEN HAVE TIME
	// might want to use clock callback to fire this instead of checking every time??
	// Check if we need to resend any ack packets
	for ( auto itr = mClientInfos.begin(); itr != mClientInfos.end(); ++itr )
	{
		ClientInfo& info = itr->second;
		PacketWriter writer;

		// Send packet with acknowledgment info to let client know we got the packet
		if ( info.PacketsToAck.size() )
		{
			SendData( writer, info.Address, SENDOPT_NONE );
		}

		for ( auto jtr = itr->second.PacketsNeedingAck.begin(); jtr != itr->second.PacketsNeedingAck.end(); ++jtr )
		{
			// If it's been a while and still haven't heard back resend packet to client
			double now = mNetClock->GetElapsedTime( Clock::TIME_MILLI );
			double diff = now - (*jtr)->TimeLastSent;
			if ( diff > mReliableResendTimeout )
			{
				if ( VerboseDebugMsg )
				{
					ConsolePrintf( C_FG_LIGHT_BLUE, "<<<<< " );
					ConsolePrintf( "Resending packet %u to %u\n", (*jtr)->PacketID, itr->first );
				}
				int requiredSize = (*jtr)->Packet.DataLength + sizeof( PacketHeader );
				ClientInfo& info = itr->second;
				// Fill in header
				PacketHeader header;
				header.Timestamp = now;
				header.PacketID = (*jtr)->PacketID;
				header.Flags = (*jtr)->Flags;
				info.LastSendTime = header.Timestamp;
				// Packets we need to ack
				header.PacketAcks = 0;

				// Write header
				memcpy( mSendPacket.Data, &header, sizeof( PacketHeader ) );
				// Write user data
				memcpy( mSendPacket.Data + sizeof( PacketHeader ), (*jtr)->Packet.Data, (*jtr)->Packet.DataLength );

				mSendPacket.DataLength = requiredSize;
				mSendPacket.Address = info.Address;

				// Send packet
				NetManager::udpSendPacket( mSock, mSendPacket );
				++mTotalPacketsSent;
			}
		}
	}

	// Call callbacks for new connections
	if ( mClientConnectCB )
	{
		for ( uint32 i = 0; i < mNewClients.size(); ++i )
		{
			ClientInfo& info = mClientInfos[ mNewClients[i] ];
			// Notify new connection to client code
			mClientConnectCB( mNewClients[i], info.Address );
		}
	}
	mNewClients.clear();

	// Call callbacks for dead connections
	if ( mClientDisconnectCB )
	{
		for ( uint32 i = 0; i < mDeadClients.size(); ++i )
		{
			ClientInfo& info = mClientInfos[ mDeadClients[i] ];
			// Notify lost connection to client code		
			mClientDisconnectCB( mDeadClients[i], info.Address );
		}
	}
	mDeadClients.clear();
}
//---------------------------------------
LocalClient& NetSession::CreateLocalClient()
{
	mLocalClient.SetSession( this );
	//mLocalClient.mID = IdFromAddres( mSock->Address );
	return mLocalClient;
}
//---------------------------------------
bool NetSession::PacketIsReady()
{
	for ( auto itr = mClientInfos.begin(); itr != mClientInfos.end(); ++itr )
	{
		if ( itr->second.IsPacketReady() )
			return true;
	}
	return false;
}
//---------------------------------------
bool NetSession::PacketIsReady( clientID_t clientID )
{
	return mClientInfos[ clientID ].IsPacketReady();
}
//---------------------------------------
void NetSession::ReceiveData( PacketReader& reader, NetClient& sender )
{
	for ( auto itr = mClientInfos.begin(); itr != mClientInfos.end(); ++itr )
	{
		if ( itr->second.IsPacketReady() )
		{
			ReceiveData( reader, itr->first );
			sender.mID = itr->first;
			sender.Address = itr->second.Address;
			break;
		}
	}
}
//---------------------------------------
void NetSession::ReceiveData( PacketReader& reader, clientID_t clientID )
{
	ClientInfo& info = mClientInfos[ clientID ];
	udpPacket* packet = info.PacketQueue.front();

	// Give user data to client
	reader.CopyDataFrom( packet->Data, packet->DataLength );
	reader.Timestamp = packet->Timestamp;

	// Remove packet from queue
	info.PacketQueue.pop();
	delete packet;
}
//---------------------------------------
void NetSession::SendData( PacketWriter& data, IPaddress& addr, int opts, bool clearOnSend )
{

	/* Packet Structure
	 * [Header][Acks][UserData]
	 */
	ClientInfo& info = mClientInfos[ IdFromAddress( addr ) ];
	PacketHeader header;

	// In case address is not current (like on first sendto)
	info.Address = addr;

	++mTotalPacketsSent;

	// Resize if needed
	int requiredSize = data.Size() + sizeof( PacketHeader ) + info.PacketsToAck.size() * sizeof( packetID_t );
	if ( mSendPacket.MaxDataLength < requiredSize )
	{
		ConsolePrintf( C_FG_AQUA, ">>>>> " );
		ConsolePrintf( "Resizing out packet %s -> %s\n"
			, ByteDisplay( mSendPacket.MaxDataLength ).ToString()
			, ByteDisplay( requiredSize ).ToString() );
		mSendPacket.Resize( requiredSize );
	}

	if ( requiredSize > mRecvPacket.MaxDataLength )
	{
		ConsolePrintf( CONSOLE_WARNING, "Sending pack of size %s will fail. Max size=%s\n"
			, ByteDisplay( requiredSize ).ToString()
			, ByteDisplay( mRecvPacket.MaxDataLength ).ToString() );
	}
		
	// Fill in header
	double now = mNetClock->GetElapsedTime( Clock::TIME_MILLI );
	//double diff = now - info.LastSendTime;
	// Sync time every sec
	//if ( diff > 500 )
	//	opts |= SENDOP_TIMESYNC;

	header.Timestamp = now;
	header.PacketID = ++info.LastSendPacketID;
	header.Flags = 0;
	/*
	if ( IsBitSet( opts, 0 ) )
		SetBit( header.Flags, 0 );	// Packet arrives in-order

	if ( IsBitSet( opts, 1 ) )
		SetBit( header.Flags, 1 );	// Packet is reliable

	if ( IsBitSet( opts, 2 ) )
		SetBit( header.Flags, 2 );	// Packet is connection request

	if ( IsBitSet( opts, 3 ) )
		SetBit( header.Flags, 3 );	// Packet is accepting connection request

	if ( IsBitSet( opts, 4 ) )
		SetBit( header.Flags, 4 );	// Packet is informing disconnect
	*/
	header.Flags = opts;

	// Record last time we sent a packet
	info.LastSendTime = header.Timestamp;

	// If this packet is to be acknowledge, we need to record the data
	//  in case we need to resend it
	if ( opts & SENDOPT_RELIABLE )
	{
//		ConsolePrintf( "Packet %u is RELIABLE. Coping data...\n", header.PacketID );

		AckInfo* ackInfo = new AckInfo();
		
		// Store data in case we need to resend it
		ackInfo->PacketID = header.PacketID;
		ackInfo->Packet.Resize( data.Size() );
		memcpy( ackInfo->Packet.Data, data.Data(), data.Size() );
		ackInfo->Packet.DataLength = data.Size();
		ackInfo->TimeLastSent = header.Timestamp;
		ackInfo->Flags = header.Flags;
		info.PacketsAcked[ header.PacketID ] = false;

		// Add to packets waiting for acknowledgment
		info.PacketsNeedingAck.push_back( ackInfo );
	}
	// Packets we received but need to acknowledge
	header.PacketAcks = info.PacketsToAck.size();

	// Write header
	memcpy( mSendPacket.Data, &header, sizeof( PacketHeader ) );

	// Write acknowledgments (if any)
	if ( header.PacketAcks > 0 )
	{
		memcpy( mSendPacket.Data + sizeof( PacketHeader ), info.PacketsToAck.data(), header.PacketAcks * sizeof( packetID_t ) );
		info.PacketsToAck.clear();
	}

	// Write user data
	memcpy( mSendPacket.Data + sizeof( PacketHeader ) + header.PacketAcks * sizeof( packetID_t ), data.Data(), data.Size() );

	//mSendPacket.Data = (uint8*)data.Data();
	mSendPacket.DataLength = requiredSize;
	mSendPacket.Address = addr;

	// Simulated packet loss
	if ( mPacketLoss && ( rand() % 100 ) <= mPacketLoss )
	{
		mSendPacket.Status = data.Size();
		if ( VerboseDebugMsg )
		{
			ConsolePrintf( C_FG_LIGHT_YELLOW, ">>>>> " );
			ConsolePrintf( "Simulating packet loss\n" );
		}
	}
	// Send packet
	else
	{
		if ( VerboseDebugMsg )
		{
			ConsolePrintf( C_FG_GREEN, "<<<<< " );
			ConsolePrintf( "Sent packet %u\n", header.PacketID );
		}
		NetManager::udpSendPacket( mSock, mSendPacket );
	}

	// Clear writer after send
	if ( clearOnSend )
	{
		data.Clear();
	}
}
//---------------------------------------
void NetSession::SendData( PacketWriter& data, int opts )
{
	for ( auto itr = mClientInfos.begin(); itr != mClientInfos.end(); ++itr )
	{
		SendData( data, itr->second.Address, opts, false );
	}
	data.Clear();
}
//---------------------------------------
void NetSession::SendConnectMessage( IPaddress& addr )
{
	PacketWriter _empty;
	SendData( _empty, addr, SENDOPT_CONNECT_REQUEST );
}
//---------------------------------------
void NetSession::SendAcceptMessage( IPaddress& addr )
{
	PacketWriter _empty;
	SendData( _empty, addr, SENDOPT_CONNECT_ACCEPT | SENDOP_TIMESYNC );
}
//---------------------------------------
void NetSession::DropClient( clientID_t clientID )
{
	auto itr = mClientInfos.find( clientID );
	if ( itr != mClientInfos.end() )
	{
		ConsolePrintf( ">>>>> Removed client %u\n", itr->first );
		PacketWriter _empty;
		SendData( _empty, itr->second.Address, SENDOP_DISCONNECT );
		mDeadClients.push_back( clientID );
		mClientInfos.erase( itr );
	}
}
//---------------------------------------
void NetSession::DropAllClients()
{
	PacketWriter _empty;
	for ( auto itr = mClientInfos.begin(); itr != mClientInfos.end(); ++itr )
	{
		ConsolePrintf( ">>>>> Removed client %u\n", itr->first );
		SendData( _empty, itr->second.Address, SENDOP_DISCONNECT );
		mDeadClients.push_back( itr->first );
	}
	mClientInfos.clear();
}
//---------------------------------------