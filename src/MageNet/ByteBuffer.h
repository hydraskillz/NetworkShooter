/*
 * Author      : Matthew Johnson
 * Date        : 20/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class ByteBuffer
	{
	protected:
		ByteBuffer();
		ByteBuffer( const ByteBuffer& other );
		~ByteBuffer();

	//public:
		void WriteBytes( const uint8* bytes, int num );
		void ReadBytes( uint8* bytes, int num );
	public:
		void Clear();
		// Copy byte array to this buffer
		void CopyDataFrom( const uint8* bytes, int num );
		const uint8* Data();
		int Size();
		
	protected:
		std::vector< uint8 > mBuffer;
		int mPos;
	};

}