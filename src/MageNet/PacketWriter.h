/*
 * Author      : Matthew Johnson
 * Date        : 20/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class PacketWriter
		: public ByteBuffer
	{
	public:
		PacketWriter();
		~PacketWriter();

		template< typename T >
		void Write( T value )
		{
			WriteBytes( (const uint8*)&value, sizeof( T ) );
		}

		// Write null terminated string
		void WriteString( const char* strz )
		{
			WriteBytes( (const uint8*)strz, strlen( strz ) + 1 );
		}
	};

}