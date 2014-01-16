#include "NetLib.h"

using namespace mage;

//---------------------------------------
PacketReader::PacketReader()
{}
//---------------------------------------
PacketReader::~PacketReader()
{}
//---------------------------------------
char* PacketReader::ReadString()
{
	char* strz = 0;
	int npos = mBuffer.size();
	for ( uint32 i = mPos; i < mBuffer.size(); ++i )
	{
		if ( mBuffer[i] == 0 )
		{
			npos = i;
			break;
		}
	}
	if ( npos == mBuffer.size() )
	{
		ConsolePrintf( CONSOLE_WARNING, "Buffer read fail: ReadString() no null terminator found!\n" );
	}
	else
	{
		strz = new char[ npos+1 ];
		ReadBytes( (uint8*)strz, (npos+1)-mPos );
	}
	return strz;
}
//---------------------------------------
char* PacketReader::ReadString( char* buff, int size )
{
	int npos = mBuffer.size();
	for ( uint32 i = mPos; i < mBuffer.size(); ++i )
	{
		if ( mBuffer[i] == 0 )
		{
			npos = i;
			break;
		}
	}
	if ( npos == mBuffer.size() )
	{
		ConsolePrintf( CONSOLE_WARNING, "Buffer read fail: ReadString() no null terminator found!\n" );
		buff[0] = 0;
	}
	else
	{
		int numBytes = (npos+1)-mPos;
		if ( numBytes >= size )
		{
			ConsolePrintf( CONSOLE_WARNING, "Buffer read fail: ReadString() buffer size exceeded!\n" );
			numBytes = size - 1;
		}
		ReadBytes( (uint8*)buff, numBytes );
	}
	return buff;
}
//---------------------------------------