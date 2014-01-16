/*
 * Author      : Matthew Johnson
 * Date        : 21/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class LocalClient
		: public NetClient
	{
	public:
		LocalClient();
		~LocalClient();

		bool IsDataReady();
		void ReceiveData( PacketReader& reader, NetClient& sender );
		// Send data to all netclients
		void SendData( PacketWriter& writer, SendDataOpts opts=SENDOPT_NONE );
		// Send data to specific netclient
		void SendData( PacketWriter& writer, IPaddress& to, SendDataOpts opts=SENDOPT_NONE );
		// Establish a connection to a server
		IPaddress ConnectTo( const char* ip, uint16 port );
		// Disconnect if connected to server
		void Disconnect();

	protected:
		bool mDataReady;
	};

}