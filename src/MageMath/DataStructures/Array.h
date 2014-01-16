/*
 * Author      : Matthew Johnson
 * Date        : 20/Dec/2012
 * Description :
 *   Fixed size managed array.
 */
#pragma once

#include <memory>

namespace mage
{

	template< int DIM, typename T >
	class Array
	{
	public:

		//---------------------------------------
		// Default constructor
		Array()
		{}

		//---------------------------------------
		// Copy constructor
		Array( const Array& other )
		{
			// Incase T is not a basic type we cannot just memcpy the array
			//memcpy( mArray, other.mArray, DIM );
			for ( int i = 0; i < DIM; ++i )
			{
				mArray[ i ] = other.mArray[ i ];
			}
		}

		//---------------------------------------
		// Deconstructor
		~Array()
		{}

		//---------------------------------------
		// Accessors
		T operator[]( int i ) const
		{
			return mArray[ i ];
		}
		T& operator[]( int i )
		{
			return mArray[ i ];
		}
		int Size() const
		{
			return DIM;
		}
		//---------------------------------------

		//---------------------------------------
		// Assignment
		Array& operator=( const Array& other )
		{
			memcpy( mArray, other.mArray, sizeof( T ) * DIM );
			//for ( int i = 0; i < DIM; ++i )
			//{
			//	mArray[ i ] = other.mArray[ i ];
			//}
			return *this;
		}
		//---------------------------------------

		//---------------------------------------
		// Boolean operations
		bool operator==( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) == 0; }
		bool operator!=( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) != 0; }
		bool operator>=( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) >= 0; }
		bool operator<=( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) <= 0; }
		bool operator> ( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) >  0; }
		bool operator< ( const Array& other ) const { return memcmp( mArray, other.mArray, sizeof( T ) * DIM ) <  0; }
		//---------------------------------------

	protected:
		T mArray[ DIM ];

	};

}