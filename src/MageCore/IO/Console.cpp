#include "CoreLib.h"

using namespace mage;

static uint16 DEFAULT_STYLE = C_FG_WHITE | C_BG_BLACK;
static Mutex gConsoleMutex;

//---------------------------------------
void ConsolePrintf( uint16 style, const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	SetConsoleStyle( style == CONSOLE_DEFAULT ? DEFAULT_STYLE : style );
	va_list vargs;
	va_start( vargs, fmt );
	vprintf( fmt, vargs );
	va_end( vargs );
	SetConsoleStyle( DEFAULT_STYLE );
}
//---------------------------------------
void ConsolePrintf( const char* fmt, ... )
{
	CriticalBlock( gConsoleMutex );

	va_list vargs;
	va_start( vargs, fmt );
	vprintf( fmt, vargs );
	va_end( vargs );
}
//---------------------------------------
void ConsoleClear()
{
	system( "cls" );
}
//---------------------------------------
void ConsoleRender( void* screen )
{}
//---------------------------------------
void ConsoleScroll( int dir )
{}
//---------------------------------------
void ConsoleToggleVisibility()
{}
//---------------------------------------
bool IsConsoleVisible()
{
	return false;
}
//---------------------------------------
bool ConsoleInput( int code, int mod )
{
	return false;
}
//---------------------------------------
bool ConsoleInit()
{
	return true;
}
//---------------------------------------
void ConsoleCreate( float width, float height )
{}
//---------------------------------------
void* GetGameConsole()
{
	return NULL;
}
//---------------------------------------

//---------------------------------------
void SetConsoleDefaultStyle( uint16 style )
{
	DEFAULT_STYLE = style;
	SetConsoleStyle( DEFAULT_STYLE );
}
//---------------------------------------