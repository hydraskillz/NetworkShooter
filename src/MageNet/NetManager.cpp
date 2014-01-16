#include "NetLib.h"

#include "NetSystem.h"

using namespace mage;

//---------------------------------------
namespace mage
{

	struct Socket
		: GenericSocket
	{
		SOCKET Sock;
	};

	struct _SocketArray
	{
		std::vector< Socket* > Sockets;
		int NumSockets() const
		{
			return (int) Sockets.size();
		}
	};

	struct tcpSocket
		: Socket
	{
		IPaddress RemoteAddress;
		IPaddress LocalAddress;
	};

	struct udpSocket
		: GenericSocket
	{
		SOCKET Sock;
		IPaddress Address;
	};

	bool SocketIsReady( SOCKET sock )
	{
		timeval tv;
		fd_set mask;
		bool ready = false;

		do 
		{
			Net_SetLastError( 0 );

			FD_ZERO( &mask );
			FD_SET( sock, &mask );

			tv.tv_sec = 0;
			tv.tv_usec = 0;

			ready = select( sock + 1, &mask, 0, 0, &tv ) == 1;
		} while ( Net_GetLastError() == EINTR );

		return ready;
	}
	
}
//---------------------------------------


//---------------------------------------
int NetManager::mNetInit = 0;
//---------------------------------------


//---------------------------------------
NetManager::NetManager()
{}
//---------------------------------------
NetManager::~NetManager()
{}
//---------------------------------------
bool NetManager::Init()
{
	if ( mNetInit == 0 )
	{
#ifdef _WIN32_SOCKETS
		WSADATA wsaData;

		if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
		{
			ConsolePrintf( CONSOLE_ERROR, "Failed to init Winsock 2.2\n" );
			return false;
		}
#endif
	}
	++mNetInit;	// Count how many times the net is initialized. you will need to quit it this many times
	return true;
}
//---------------------------------------
bool NetManager::Quit()
{
	if ( mNetInit == 1 )
	{
#ifdef _WIN32_SOCKETS
		WSACleanup();
#endif
	}
	--mNetInit;
	return mNetInit == 0;
}
//---------------------------------------
bool NetManager::ResolveHost( IPaddress& address, const char* host, uint16 port )
{
	if ( host == 0 )
	{
		address.Host = INADDR_ANY;
	}
	else
	{
		address.Host = inet_addr( host );
		if ( address.Host == INADDR_NONE )
		{
			hostent* h;

			h = gethostbyname( host );
			if ( h )
			{
				memcpy( &address.Host, h->h_addr, h->h_length );
			}
			else
			{
				ConsolePrintf( CONSOLE_ERROR, "Failed to resolve host '%s:%u'\n", host, port );
				return false;
			}
		}
	}
	address.Port = port;
	return true;
}
//---------------------------------------
int NetManager::CheckSockets( SocketArray sockets, uint32 timeoutMS )
{
	SOCKET maxSock = 0;
	int _ret;
	timeval tv;
	fd_set mask;

	// Get largest socket
	for ( int i = sockets->NumSockets() - 1; i >= 0; --i )
	{
		if ( sockets->Sockets[i]->Sock > maxSock )
		{
			maxSock = sockets->Sockets[i]->Sock;
		}
	}

	// Check sockets for data
	do 
	{
		Net_SetLastError( 0 );

		// Set socket masks
		FD_ZERO( &mask );
		for ( int i = sockets->NumSockets() - 1; i >= 0; --i )
		{
			FD_SET( sockets->Sockets[i]->Sock, &mask );
		}

		// Set timeout
		tv.tv_sec  = timeoutMS / 1000;
		tv.tv_usec = ( timeoutMS % 1000 ) * 1000;

		_ret = select( maxSock + 1, &mask, NULL, NULL, &tv );
	} while ( Net_GetLastError() == EINTR );

	// Mark all sockets that are ready
	if ( _ret > 0 )
	{
		for ( int i = sockets->NumSockets() - 1; i >= 0; --i )
		{
			if ( FD_ISSET( sockets->Sockets[i]->Sock, &mask ) )
			{
				sockets->Sockets[i]->Ready = 1;
			}
		}
	}
	return _ret;
}
//---------------------------------------
int NetManager::GetHostName( char* name, int len )
{
	return gethostname( name, len );
}
//---------------------------------------


//---------------------------------------
udpSocket_t NetManager::udpOpenPort( uint16 port )
{
	udpSocket_t sock;
	sockaddr_in sock_addr;
	socklen_t sock_len;

	sock = new udpSocket();

	// Zero mem
	memset( sock, 0, sizeof( *sock ) );
	memset( &sock_addr, 0, sizeof( sock_addr ) );

	// Open socket
	sock->Sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( sock->Sock == INVALID_SOCKET )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to create socket.\n" );
		udpCloseSocket( sock );
		return 0;
	}

	// Bind
	sock_addr.sin_family      = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port        = port;

	sock_len = sizeof( sock_addr );

	if ( bind( sock->Sock, (sockaddr*)&sock_addr, sock_len ) == SOCKET_ERROR )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to bind socket to port.\n" );
		udpCloseSocket( sock );
		return 0;
	}

	// Set socket info
	if ( getsockname( sock->Sock, (sockaddr*)&sock_addr, &sock_len ) < 0 )
	{
		ConsolePrintf( CONSOLE_ERROR, "Failed to get socket address.\n" );
		udpCloseSocket( sock );
		return 0;
	}

	sock->Address.Host = sock_addr.sin_addr.s_addr;
	sock->Address.Port = sock_addr.sin_port;

#ifdef SO_BROADCAST
	int x = 1;
	setsockopt( sock->Sock, SOL_SOCKET, SO_BROADCAST, (char*)&x, sizeof( x ) );
#endif

	return sock;
}
//---------------------------------------
void NetManager::udpCloseSocket( udpSocket_t& sock )
{
	if ( sock )
	{
		if ( sock->Sock != INVALID_SOCKET )
			closesocket( sock->Sock );
		Delete0( sock );
	}
}
//---------------------------------------
int NetManager::udpSendPacket( udpSocket_t sock, const udpPacket& packet )
{
	sockaddr_in sock_addr;
	socklen_t sock_len;
	int status;
	int sent = 0;

	sock_addr.sin_addr.s_addr = packet.Address.Host;
	sock_addr.sin_port        = packet.Address.Port;
	sock_addr.sin_family      = AF_INET;

	sock_len = sizeof( sock_addr );
	status = sendto( sock->Sock, (const char*) packet.Data, packet.DataLength, 0,
					 (sockaddr*)&sock_addr, sock_len );

	if ( status >= 0 )
	{
		const_cast< udpPacket& >( packet ).Status = status;
		++sent;
	}

	return sent;
}
//---------------------------------------
int NetManager::udpRecvPacket( udpSocket_t sock, udpPacket& packet )
{
	sockaddr_in sock_addr;
	socklen_t sock_len;
	int numrecv = 0;

	if ( SocketIsReady( sock->Sock ) )
	{
		sock_len = sizeof( sock_addr );
		packet.Status = recvfrom( sock->Sock, (char*) packet.Data, packet.MaxDataLength, 0,
			                       (sockaddr*)&sock_addr, &sock_len );

		if ( packet.Status >= 0 )
		{
			packet.DataLength = packet.Status;
			packet.Address.Host = sock_addr.sin_addr.s_addr;
			packet.Address.Port = sock_addr.sin_port;

			++numrecv;
		}
		else
			packet.DataLength = 0;
	}

	return numrecv;
}
//---------------------------------------