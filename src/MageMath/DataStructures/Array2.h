/*
 * Author      : Matthew Johnson
 * Date        : 26/Dec/2012
 * Description :
 *   Managed 2D fixed sized array.
 */
 
#pragma once

#include <memory>

namespace mage
{

	template< int ROWS, int COLS, typename T >
	class Array2
	{
	public:
		// Constructors
		Array2();
		Array2( const Array2& array2 );
		~Array2();

		// Accessors
		inline const T* operator[]( int row ) const;
		inline T* operator[]( int row );
		inline T operator()( int row, int col ) const;
		inline T& operator()( int row, int col );

		// Assignment
		Array2& operator=( const Array2& array2 );

		// Comparison
		bool operator==( const Array2& array2 ) const;
		bool operator!=( const Array2& array2 ) const;
		bool operator< ( const Array2& array2 ) const;
		bool operator<=( const Array2& array2 ) const;
		bool operator> ( const Array2& array2 ) const;
		bool operator>=( const Array2& array2 ) const;

	protected:
		enum { SIZE = ROWS * COLS };
		T mArray[ SIZE ];
	};

	//---------------------------------------
	// Implementation
	//---------------------------------------

	// Macro for required template
#	define A2_TMPL template< int ROWS, int COLS, typename T >
#	define A2 Array2< ROWS, COLS, T >

	//---------------------------------------
	// Constructors
	A2_TMPL A2::Array2()
	{}
	
	A2_TMPL A2::Array2( const Array2& array2 )
	{
		for ( int i = 0; i < SIZE; ++i )
		{
			mArray[i] = array2.mArray[i];
		}
	}
	
	A2_TMPL A2::~Array2()
	{}
	//---------------------------------------

	//---------------------------------------
	// Accessors
	A2_TMPL const T* A2::operator[]( int row ) const
	{
		return &mArray[ COLS * row ];
	}
	A2_TMPL T* A2::operator[]( int row )
	{
		return &mArray[ COLS * row ];
	}
	A2_TMPL T A2::operator()( int row, int col ) const
	{
		return mArray[ col + COLS * row ];
	}
	A2_TMPL T& A2::operator()( int row, int col )
	{
		return mArray[ col + COLS * row ];
	}
	//---------------------------------------

	//---------------------------------------
	// Assignment
	A2_TMPL A2& A2::operator=( const Array2& array2 )
	{
		for ( int i = 0; i < SIZE; ++i )
		{
			mArray[i] = array2.mArray[i];
		}
		return *this;
	}
	//---------------------------------------

	//---------------------------------------
	// Comparison
	A2_TMPL bool A2::operator==( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) == 0; }
	A2_TMPL bool A2::operator!=( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) != 0; }
	A2_TMPL bool A2::operator< ( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) <  0; }
	A2_TMPL bool A2::operator<=( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) <= 0; }
	A2_TMPL bool A2::operator> ( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) >  0; }
	A2_TMPL bool A2::operator>=( const Array2& array2 ) const { return memcmp( mArray, array2.mArray, SIZE * sizeof( T ) ) >= 0; }
	//---------------------------------------

#	undef A2_TMPL
#	undef A2
}