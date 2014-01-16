/*
 * Author      : Matthew Johnson
 * Date        : 20/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class PacketReader
		: public ByteBuffer
	{
	public:
		PacketReader();
		~PacketReader();

		template< typename T >
		T Read()
		{
			T value;
			if ( mPos + sizeof( T ) > mBuffer.size() )
			{
				ConsolePrintf( CONSOLE_WARNING, "Buffer read fail: not enough to read!\n" );
			}
			else
			{
				ReadBytes( (uint8*)&value, sizeof( T ) );
			}
			return value;
		}

		// Get a null terminated string (dynamic buffer)
		char* ReadString();
		// Get a null terminated string (static buffer)
		char* ReadString( char* buff, int size );

		double Timestamp;	// Time this data was received (ms)
	};

}