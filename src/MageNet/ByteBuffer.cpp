#include "NetLib.h"

using namespace mage;

//---------------------------------------
ByteBuffer::ByteBuffer()
	: mPos( -1 )
{}
//---------------------------------------
ByteBuffer::ByteBuffer( const ByteBuffer& other )
{
	mBuffer = other.mBuffer;
	mPos = other.mPos;
}
//---------------------------------------
ByteBuffer::~ByteBuffer()
{}
//---------------------------------------
void ByteBuffer::WriteBytes( const uint8* bytes, int num )
{
	for ( int i = 0; i < num; ++i )
		mBuffer.push_back( bytes[i] );
}
//---------------------------------------
void ByteBuffer::ReadBytes( uint8* bytes, int num )
{
	memcpy( bytes, &mBuffer[mPos], num );
	mPos += num;
}
//---------------------------------------
void ByteBuffer::Clear()
{
	mBuffer.clear();
}
//---------------------------------------
void ByteBuffer::CopyDataFrom( const uint8* bytes, int num )
{
	mBuffer.clear();
	mBuffer.resize( num );
	memcpy( mBuffer.data(), bytes, num );
	mPos = 0;
}
//---------------------------------------
const uint8* ByteBuffer::Data()
{
	return mBuffer.data();
}
//---------------------------------------
int ByteBuffer::Size()
{
	return mBuffer.size();
}
//---------------------------------------