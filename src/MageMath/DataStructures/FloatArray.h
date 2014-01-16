/*
 * Author      : Matthew Johnson
 * Date        : 20/Dec/2012
 * Description :
 *  Managed arrays of floats.
 */
#pragma once

#include "Array.h"

namespace mage
{

	class Float2
		: public Array< 2, float >
	{
	public:
		//---------------------------------------
		// Constructors
		Float2()
		{}
		Float2( float f1, float f2 )
		{
			mArray[0] = f1;
			mArray[1] = f2;
		}
		Float2( const Float2& other )
		{
			mArray[0] = other.mArray[0];
			mArray[1] = other.mArray[1];
		}
		//---------------------------------------

		//---------------------------------------
		// Destructor
		~Float2()
		{}
		//---------------------------------------

		//---------------------------------------
		// Assignment
		Float2& operator=( const Float2& other )
		{
			mArray[0] = other.mArray[0];
			mArray[1] = other.mArray[1];
			return *this;
		}
		//---------------------------------------
	};

	class Float3
		: public Array< 3, float >
	{
	public:
		//---------------------------------------
		// Constructors
		Float3()
		{}
		Float3( float f1, float f2, float f3 )
		{
			mArray[0] = f1;
			mArray[1] = f2;
			mArray[2] = f3;
		}
		Float3( const Float3& other )
		{
			mArray[0] = other.mArray[0];
			mArray[1] = other.mArray[1];
			mArray[2] = other.mArray[2];
		}
		//---------------------------------------

		//---------------------------------------
		// Destructor
		~Float3()
		{}
		//---------------------------------------

		//---------------------------------------
		// Assignment
		Float3& operator=( const Float3& other )
		{
			mArray[0] = other.mArray[0];
			mArray[1] = other.mArray[1];
			mArray[2] = other.mArray[2];
			return *this;
		}
		//---------------------------------------
	};

	class Float4
		: public Array< 4, float >
	{
	public:
		//---------------------------------------
		// Constructors
		Float4()
		{
			mArray[0] = 0.0f;
			mArray[1] = 0.0f;
			mArray[2] = 0.0f;
			mArray[3] = 0.0f;
		}
		Float4( float f1, float f2, float f3, float f4 )
		{
			mArray[0] = f1;
			mArray[1] = f2;
			mArray[2] = f3;
			mArray[3] = f4;
		}
		Float4( const Float4& other )
		{
			mArray[0] = other.mArray[0];
			mArray[1] = other.mArray[1];
			mArray[2] = other.mArray[2];
			mArray[3] = other.mArray[3];
		}
		//---------------------------------------

		//---------------------------------------
		// Destructor
		~Float4()
		{}
		//---------------------------------------

		//---------------------------------------
		// Assignment
//		Float4& operator=( const Float4& other )
//		{
//			mArray[0] = other.mArray[0];
//			mArray[1] = other.mArray[1];
//			mArray[2] = other.mArray[2];
//			mArray[3] = other.mArray[3];
//			return *this;
//		}
		//---------------------------------------
	};


	// ... More as needed ... (probably not)

}