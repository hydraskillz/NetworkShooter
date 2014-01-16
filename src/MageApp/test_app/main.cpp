#include <MageApp.h>

using namespace mage;

void Update( float dt );

int main( int argc, char** argv )
{
	InitApp( "The Test App", 800, 600 );

	RegisterUpdateFn( Update );

	Run();

	return 0;
}

void Update( float dt )
{
	printf( "Update1\n" );
}