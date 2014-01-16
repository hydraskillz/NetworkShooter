#include <MageMath.h>
#include <MageCore.h>
#include <MageNet.h>

using namespace mage;

/* This is a test console app for udp net interface.
 */
int main( int argc, char** argv )
{
	bool quit = false;
	PacketReader reader;
	PacketWriter writer;
	char buff[512];
	NetSession session;
	LocalClient server;
	LocalClient sender;
	int frame = 0;

	// Init
	session.OpenPort( 5000 );
	server = session.CreateLocalClient();

	//session.SetPacketLoss( 50 );


	// Send stuff
	ConsolePrintf( "Server is running.\n" );
	while ( !quit )
	{
		session.OnUpdate();
		while ( server.IsDataReady() )
		{
			server.ReceiveData( reader, sender );

			frame = reader.Read< int >();
			ConsolePrintf( "RECV: %d\n", frame );
			//quit = !strcmp( buff, "quit" );

			sprintf( buff, "echo: %d", frame );
			//ConsolePrintf( "Reply: %s\n", buff );
			writer.WriteString( buff );
			server.SendData( writer );
		}
	}


	// Quit
	

	return 0;
}