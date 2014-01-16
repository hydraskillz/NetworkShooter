/*
 * Author      : Matthew Johnson
 * Date        : 20/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class NetManager
	{
	private:
		NetManager();
		~NetManager();
	public:
		// Init net framework
		static bool Init();

		// Shutdown and cleanup
		static bool Quit();

		//---------------------------------------
		// General Network
		//---------------------------------------

		// Resolve name/port to IPaddress
		static bool ResolveHost( IPaddress& address, const char* host, uint16 port );

		/**Get local addresses up to max.
		 * If max < 0 get all local addresses
		 * Returns number address found
		 */
		static int GetLocalAddresses( std::vector< IPaddress >& addresses, int max=-1 );

		/**Check to see if data is ready on the given sockets.
		 * If timeout is 0 this function returns after a single check.
		 * Otherwise returns when data is ready or the timeout is reached.
		 * Returns the number of sockets ready for reading.
		 */
		static int CheckSockets( SocketArray sockets, uint32 timeoutMS );

		// Returns true if the socket is valid and has data waiting
#define IsSocketReady( sock ) _IsSocketReady( (genericSocket_t) sock )
		static inline bool _IsSocketReady( genericSocket_t sock )
		{
			return ( sock && sock->Ready );
		}

		static int GetHostName( char* name, int len );


		//---------------------------------------
		// UDP Network
		//---------------------------------------

		static udpSocket_t udpOpenPort( uint16 port );
		static void udpCloseSocket( udpSocket_t& sock );
		// Sends a udpPacket over the given socket. The 'Status' field of udpPacket is modified.
		static int udpSendPacket( udpSocket_t sock, const udpPacket& packet );
		// Receive a udpPacket over the given socket. Returns the number of packets received.
		static int udpRecvPacket( udpSocket_t sock, udpPacket& packet );

	private:
		static int mNetInit;
	};

}