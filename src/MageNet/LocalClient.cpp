#include "NetLib.h"

using namespace mage;

//---------------------------------------
LocalClient::LocalClient()
{}
//---------------------------------------
LocalClient::~LocalClient()
{}
//---------------------------------------
bool LocalClient::IsDataReady()
{
	return mSession->PacketIsReady();
}
//---------------------------------------
void LocalClient::ReceiveData( PacketReader& reader, NetClient& sender )
{
	mSession->ReceiveData( reader, sender );
}
//---------------------------------------
void LocalClient::SendData( PacketWriter& writer, SendDataOpts opts )
{
	mSession->SendData( writer, opts );
}
//---------------------------------------
void LocalClient::SendData( PacketWriter& writer, IPaddress& to, SendDataOpts opts )
{
	mSession->SendData( writer, to, opts );
}
//---------------------------------------
IPaddress LocalClient::ConnectTo( const char* ip, uint16 port )
{
	IPaddress addr;
	// Listen on any port
	mSession->OpenPort( 0 );
	// Resolve the address
	NetManager::ResolveHost( addr, ip, port );
	mSession->SendConnectMessage( addr );
	return addr;
}
//---------------------------------------
void LocalClient::Disconnect()
{
	mSession->DropAllClients();
}
//---------------------------------------