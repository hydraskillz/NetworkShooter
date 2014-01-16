#include "main.h"

using namespace mage;

int main( int argc, char** argv )
{
	CommandArgs args( argc, argv );
	bool isServer = args.HasParam( "-server" );
	std::string addr = "localhost";
	args.GetArgAs( "-connect", addr );

	isServer ? InitServer() : InitClient( addr );
	
	Run();

	isServer ? OnServerExit() : OnClientExit();

	return 0;
}