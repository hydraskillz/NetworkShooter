#include <MageMath.h>
#include <MageCore.h>
#include <MageNet.h>
#include <SDL.h>

using namespace mage;

/* This is a test console app for udp net interface.
 * Type stuff to send it to the test server.
 * Type 'quit' to quit.
 */
int main( int argc, char** argv )
{
	IPaddress addr;
	bool quit = false;
	PacketWriter writer;
	PacketReader reader;
	char buff[512];
	NetSession session;
	LocalClient client;
	LocalClient sender;
	CommandArgs args( argc, argv );
	int frame = 0;
	
	// Init
	SDL_Init( SDL_INIT_TIMER );
	session.OpenPort( 0 );
	if ( args.GetArgAs( "-address", buff ) )
	{
		ConsolePrintf( "Connecting to %s\n", buff );
		NetManager::ResolveHost( addr, buff, 5000 );
	}
	else
		NetManager::ResolveHost( addr, "localhost", 5000 );
	client = session.CreateLocalClient();
	session.SetPacketLoss( 50 );

	// Send stuff
	while ( !quit )
	{
		static int ticks = SDL_GetTicks();
		if ( (SDL_GetTicks() - ticks) > (1000 / 60) )	// Run sim at 60 FPS
		{
			ticks = SDL_GetTicks();

			if ( frame % 100 == 0 )
			{
				ConsolePrintf( "Send: %d\n", frame );
				writer.Write< int >( frame );
				client.SendData( writer, addr, SENDOPT_RELIABLE );
				writer.Write< int >( frame + 1 );
				client.SendData( writer, addr );
			}
			++frame;

			session.OnUpdate();

			while ( client.IsDataReady() )
			{
				client.ReceiveData( reader, sender );
				//ConsolePrintf( "Recv:\n %s\n", reader.ReadString( buff, 512 ) );
				//ConsolePrintf( " RTT: %fs\n", session.GetAverageRTT( sender.GetID() ) );
			}
		}
	}


	// Quit

	return 0;
}