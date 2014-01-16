/*
 * Author      : Matthew Johnson
 * Date        : 26/Dec/2012
 * Description :
 *   
 */
 
#pragma once

#include "Array2.h"
#include "Vector3.h"

#include <math.h>

namespace mage
{

	template< typename TReal >
	class Matrix3
		: public Array2< 3, 3, TReal >
	{
	public:
		// Constructors
		Matrix3( bool zero = false );
		Matrix3( const Matrix3& mat3 );
		Matrix3( TReal m00, TReal m01, TReal m02,
				 TReal m10, TReal m11, TReal m12,
				 TReal m20, TReal m21, TReal m22 );
		Matrix3( const TReal array[9], bool rowMajor=true );
		Matrix3( const Vector3< TReal > u,
				 const Vector3< TReal > v,
				 const Vector3< TReal > w,
				 bool columns );
		Matrix3( TReal m00, TReal m11, TReal m22 );
		Matrix3( const Vector3< TReal > axis, TReal angleRad );

		// Assignment
		Matrix3& operator=( const Matrix3& mat3 );

		// Create special Matricies
		Matrix3& Zero();
		Matrix3& Identity();
		Matrix3& Diagonal( TReal m00, TReal m11, TReal m22 );
		Matrix3& Rotation( const Vector3< TReal >& axis, TReal angleRad );
		Matrix3& Orthonormalize();
		

		// Arithmetic operations
		Matrix3 operator+( const Matrix3& mat3 ) const;
		Matrix3 operator-( const Matrix3& mat3 ) const;
		Matrix3 operator*( const TReal scalar ) const;
		Matrix3 operator/( const TReal scalar ) const;
		Matrix3 operator-() const;

		// Arithmetic modifications
		Matrix3& operator+=( const Matrix3& mat3 ) const;
		Matrix3& operator-=( const Matrix3& mat3 ) const;
		Matrix3& operator*=( const TReal scalar ) const;
		Matrix3& operator/=( const TReal scalar ) const;

		// Vector multiplication M * Vector
		Vector3< TReal > operator*( const Vector3< TReal >& v ) const;

		// Matrix multiplication
		Matrix3 operator*( const Matrix3& mat3 ) const;

		// Scalar * Matrix
		friend inline Matrix3< TReal > operator*( TReal scalar, const Matrix3< TReal >& mat3 );

		// Vector^T * Matrix
		friend inline Vector3< TReal > operator*( const Vector3< TReal >& v, const Matrix3< TReal >& mat3 );

		// Transpose
		Matrix3 Transpose() const;

		// Inverse
		Matrix3 Inverse( const TReal epsilon = 0 ) const;

		// Adjoint
		Matrix3 Adjoint() const;

		// Determinant
		TReal Determinant() const;

		// Special Matricies
		static const Matrix3 ZERO;
		static const Matrix3 IDENTITY;

	protected:
		using Array2< 3, 3, TReal >::mArray;
	};
	/*
	// Scalar * Matrix
	template< typename TReal >
	inline Matrix3< TReal > operator*( TReal scalar, const Matrix3< TReal >& mat3 );

	// Vector^T * Matrix
	template< typename TReal >
	inline Vector3< TReal > operator*( const Vector3< TReal >& v, const Matrix3< TReal >& mat3 );
	*/

	//---------------------------------------
	// Implementation
	//---------------------------------------

	// Macro for required template
#	define M3_TMPL template< typename TReal >
#	define M3 Matrix3< TReal >

	// Macros to simplify definitions
#	define M3_COPY( ARRAY ) for ( int i = 0; i < SIZE; ++i ) mArray[i] = ARRAY[i];

#	define M3_ARRAY_PRE_OP( FIRST, OP ) \
	FIRST[0] OP mArray[0], \
	FIRST[1] OP mArray[1], \
	FIRST[2] OP mArray[2], \
	FIRST[3] OP mArray[3], \
	FIRST[4] OP mArray[4], \
	FIRST[5] OP mArray[5], \
	FIRST[6] OP mArray[6], \
	FIRST[7] OP mArray[7], \
	FIRST[8] OP mArray[8]

#	define M3_ARRAY_POST_OP( SECOND, OP ) \
	mArray[0] OP SECOND[0], \
	mArray[1] OP SECOND[1], \
	mArray[2] OP SECOND[2], \
	mArray[3] OP SECOND[3], \
	mArray[4] OP SECOND[4], \
	mArray[5] OP SECOND[5], \
	mArray[6] OP SECOND[6], \
	mArray[7] OP SECOND[7], \
	mArray[8] OP SECOND[8]

#	define M3_SCALAR_PRE_OP( FIRST, OP ) \
	FIRST OP mArray[0], \
	FIRST OP mArray[1], \
	FIRST OP mArray[2], \
	FIRST OP mArray[3], \
	FIRST OP mArray[4], \
	FIRST OP mArray[5], \
	FIRST OP mArray[6], \
	FIRST OP mArray[7], \
	FIRST OP mArray[8]

#	define M3_SCALAR_POST_OP( SECOND, OP ) \
	mArray[0] OP SECOND, \
	mArray[1] OP SECOND, \
	mArray[2] OP SECOND, \
	mArray[3] OP SECOND, \
	mArray[4] OP SECOND, \
	mArray[5] OP SECOND, \
	mArray[6] OP SECOND, \
	mArray[7] OP SECOND, \
	mArray[8] OP SECOND

#	define M3_ARRAY_OP( OP, ARRAY ) \
	for ( int i = 0; i < SIZE; ++i ) mArray[i] OP ARRAY[i];

#	define M3_SCALAR_OP( OP, SCALAR ) \
	for ( int i = 0; i < SIZE; ++i ) mArray[i] OP SCALAR;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	M3_TMPL M3::Matrix3( bool zero )
	{
		if ( zero ) Zero();
		else        Identity();
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( const Matrix3& mat3 )
	{
		M3_COPY( mat3.mArray );
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( TReal m00, TReal m01, TReal m02,
						 TReal m10, TReal m11, TReal m12,
						 TReal m20, TReal m21, TReal m22 )
	{
		mArray[0] = m00;
		mArray[1] = m01;
		mArray[2] = m02;
		mArray[3] = m10;
		mArray[4] = m11;
		mArray[5] = m12;
		mArray[6] = m20;
		mArray[7] = m21;
		mArray[8] = m22;
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( const TReal array[9], bool rowMajor )
	{
		if ( rowMajor )
			M3_COPY( array );
		else
		{
			mArray[0] = array[0];
			mArray[1] = array[3];
			mArray[2] = array[6];
			mArray[3] = array[1];
			mArray[4] = array[4];
			mArray[5] = array[7];
			mArray[6] = array[2];
			mArray[7] = array[5];
			mArray[8] = array[8];
		}
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( const Vector3< TReal > u,
						 const Vector3< TReal > v,
						 const Vector3< TReal > w,
						 bool columns )
	{
		if ( columns )
		{
			mArray[0] = u[0];
			mArray[1] = v[0];
			mArray[2] = w[0];
			mArray[3] = u[1];
			mArray[4] = v[1];
			mArray[5] = w[1];
			mArray[6] = u[2];
			mArray[7] = v[2];
			mArray[8] = w[2];
		}
		else
		{
			mArray[0] = u[0];
			mArray[1] = u[1];
			mArray[2] = u[2];
			mArray[3] = v[0];
			mArray[4] = v[1];
			mArray[5] = v[2];
			mArray[6] = w[0];
			mArray[7] = w[1];
			mArray[8] = w[2];
		}
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( TReal m00, TReal m11, TReal m22 )
	{
		Diagonal( m00, m11, m22 );
	}
	//---------------------------------------
	M3_TMPL M3::Matrix3( const Vector3< TReal > axis, TReal angleRad )
	{
		Rotation( axis, angleRad );
	}
	//---------------------------------------

	//---------------------------------------
	// Assignment
	//---------------------------------------
	M3_TMPL M3& M3::operator=( const Matrix3& mat3 )
	{
		M3_COPY( mat3.mArray );
		return *this;
	}
	//---------------------------------------

	//---------------------------------------
	// Create special Matricies
	//---------------------------------------
	M3_TMPL M3& M3::Zero()
	{
		return Diagonal( 0, 0, 0 );
	}
	//---------------------------------------
	M3_TMPL M3& M3::Identity()
	{
		return Diagonal( 1, 1, 1 );
	}
	//---------------------------------------
	M3_TMPL M3& M3::Diagonal( TReal m00, TReal m11, TReal m22 )
	{
		mArray[0] = m00;
		mArray[1] = 0;
		mArray[2] = 0;
		mArray[3] = 0;
		mArray[4] = m11;
		mArray[5] = 0;
		mArray[6] = 0;
		mArray[7] = 0;
		mArray[8] = m22;
		return *this;
	}
	//---------------------------------------
	M3_TMPL M3& M3::Rotation( const Vector3< TReal >& axis, TReal angleRad )
	{
		TReal cs = std::cos( angleRad );
		TReal sn = std::sin( angleRad );
		TReal oneMinusCos = 1 - cs;

		TReal x2 = axis[0] * axis[0];
		TReal y2 = axis[1] * axis[1];
		TReal z2 = axis[2] * axis[2];
		TReal xy = axis[0] * axis[1] * oneMinusCos;
		TReal xz = axis[0] * axis[2] * oneMinusCos;
		TReal yz = axis[1] * axis[2] * oneMinusCos;
		TReal xs = axis[0] * sn;
		TReal ys = axis[1] * sn;
		TReal zs = axis[2] * sn;

		mArray[0] = x2 * oneMinusCos + cs;
		mArray[1] = xy - zs;
		mArray[2] = xz + ys;
		mArray[3] = xy + zs;
		mArray[4] = y2 * oneMinusCos + cs;
		mArray[5] = yz - xs;
		mArray[6] = xz - ys;
		mArray[7] = yz + xs;
		mArray[8] = z2 * oneMinusCos + cs;
		return *this;
	}
	//---------------------------------------
	M3_TMPL M3& M3::Orthonormalize()
	{
		Vector3< TReal > X = Vector3< TReal >( mArray[0], mArray[1], mArray[2] );
		Vector3< TReal > Y = Vector3< TReal >( mArray[3], mArray[4], mArray[5] );
		Vector3< TReal > Z;

		X.Normalize();
		Z = X.Cross( Y );
		Z.Normalize();
		Y = Z.Cross( X );
		Y.Normalize();

		return ( *this = M3( X, Y, Z, false ) );
	}
	//---------------------------------------
		
	//---------------------------------------
	// Arithmetic operations
	//---------------------------------------
	M3_TMPL M3 M3::operator+( const Matrix3& mat3 ) const
	{
		return M3( M3_ARRAY_POST_OP( mat3.mArray, + ) );
	}
	//---------------------------------------
	M3_TMPL M3 M3::operator-( const Matrix3& mat3 ) const
	{
		return M3( M3_ARRAY_POST_OP( mat3.mArray, - ) );
	}
	//---------------------------------------
	M3_TMPL M3 M3::operator*( const TReal scalar ) const
	{
		return M3( M3_SCALAR_PRE_OP( scalar, * ) );
	}
	//---------------------------------------
	M3_TMPL M3 M3::operator/( const TReal scalar ) const
	{
		if ( scalar != 0 )
		{
			TReal invScalar = 1 / scalar;
			return M3( M3_SCALAR_PRE_OP( invScalar, * ) );
		}
		return M3( Math< TReal >::MAX_REAL );
	}
	//---------------------------------------
	M3_TMPL M3 M3::operator-() const
	{
		return M3( M3_SCALAR_PRE_OP( -1, * ) );
	}
	//---------------------------------------

	//---------------------------------------
	// Arithmetic modifications
	//---------------------------------------
	M3_TMPL M3& M3::operator+=( const Matrix3& mat3 ) const
	{
		M3_ARRAY_OP( +=, mat3.mArray );
		return *this;
	}
	//---------------------------------------
	M3_TMPL M3& M3::operator-=( const Matrix3& mat3 ) const
	{
		M3_ARRAY_OP( -=, mat3.mArray );
		return *this;
	}
	//---------------------------------------
	M3_TMPL M3& M3::operator*=( const TReal scalar ) const
	{
		M3_SCALAR_OP( *=, scalar );
		return *this;
	}
	//---------------------------------------
	M3_TMPL M3& M3::operator/=( const TReal scalar ) const
	{
		if ( scalar != 0 )
		{
			TReal invScalar = 1 / scalar;
			M3_SCALAR_OP( *=, invScalar );
		}
		else
		{
			M3_SCALAR_OP( =, Math< TReal >::MAX_REAL );
		}
		return *this;
	}
	//---------------------------------------

	//---------------------------------------
	// Vector multiplication M * Vector
	//---------------------------------------
	M3_TMPL Vector3< TReal > M3::operator*( const Vector3< TReal >& v ) const
	{
		return Vector3< TReal >
		(
			mArray[0] * v[0] + mArray[1] * v[1] + mArray[2] * v[2],
			mArray[3] * v[0] + mArray[4] * v[1] + mArray[5] * v[2],
			mArray[6] * v[0] + mArray[7] * v[1] + mArray[8] * v[2]
		);
	}
	//---------------------------------------

	//---------------------------------------
	// Matrix multiplication
	//---------------------------------------
	M3_TMPL M3 M3::operator*( const Matrix3& mat3 ) const
	{
		return M3
		(
			mArray[0] * mat3.mArray[0] +
			mArray[1] * mat3.mArray[3] +
			mArray[2] * mat3.mArray[6],

			mArray[0] * mat3.mArray[1] +
			mArray[1] * mat3.mArray[4] +
			mArray[2] * mat3.mArray[7],

			mArray[0] * mat3.mArray[2] +
			mArray[1] * mat3.mArray[5] +
			mArray[2] * mat3.mArray[8],

			mArray[3] * mat3.mArray[0] +
			mArray[4] * mat3.mArray[3] +
			mArray[5] * mat3.mArray[6],

			mArray[3] * mat3.mArray[1] +
			mArray[4] * mat3.mArray[4] +
			mArray[5] * mat3.mArray[7],

			mArray[3] * mat3.mArray[2] +
			mArray[4] * mat3.mArray[5] +
			mArray[5] * mat3.mArray[8],

			mArray[6] * mat3.mArray[0] +
			mArray[7] * mat3.mArray[3] +
			mArray[8] * mat3.mArray[6],

			mArray[6] * mat3.mArray[1] +
			mArray[7] * mat3.mArray[4] +
			mArray[8] * mat3.mArray[7],

			mArray[6] * mat3.mArray[2] +
			mArray[7] * mat3.mArray[5] +
			mArray[8] * mat3.mArray[8]
		);
	}
	//---------------------------------------

	//---------------------------------------
	// Scalar * Matrix
	//---------------------------------------
	M3_TMPL inline M3 operator*( TReal scalar, const M3& mat3 )
	{
		return mat3 * scalar;
	}
	//---------------------------------------

	//---------------------------------------
	// Vector^T * Matrix
	//---------------------------------------
	M3_TMPL inline Vector3< TReal > operator*( const Vector3< TReal >& v, const M3& mat3 )
	{
		return Vector3< TReal >
		(
			v[0] * mat3[0][0] + v[1] * mat3[1][0] + v[2] * mat[2][0],
			v[0] * mat3[0][1] + v[1] * mat3[1][1] + v[2] * mat[2][1],
			v[0] * mat3[0][2] + v[1] * mat3[1][2] + v[2] * mat[2][2]
		);
	}
	//---------------------------------------

	//---------------------------------------
	// Transpose
	//---------------------------------------
	M3_TMPL M3 M3::Transpose() const
	{
		return M3
		(
			mArray[0],
			mArray[3],
			mArray[6],
			mArray[1],
			mArray[4],
			mArray[7],
			mArray[2],
			mArray[5],
			mArray[8]
		);
	}
	//---------------------------------------

	//---------------------------------------
	// Inverse
	//---------------------------------------
	M3_TMPL M3 M3::Inverse( const TReal epsilon ) const
	{
		Matrix3 inverse;

		// Compute adjoint
		inverse.mArray[0] = mArray[4] * mArray[8] - mArray[5] * mArray[7];
		inverse.mArray[1] = mArray[2] * mArray[7] - mArray[1] * mArray[8];
		inverse.mArray[2] = mArray[1] * mArray[5] - mArray[2] * mArray[4];
		inverse.mArray[3] = mArray[5] * mArray[6] - mArray[3] * mArray[8];
		inverse.mArray[4] = mArray[0] * mArray[8] - mArray[2] * mArray[6];
		inverse.mArray[5] = mArray[2] * mArray[3] - mArray[0] * mArray[5];
		inverse.mArray[6] = mArray[3] * mArray[7] - mArray[4] * mArray[6];
		inverse.mArray[7] = mArray[1] * mArray[6] - mArray[0] * mArray[7];
		inverse.mArray[8] = mArray[0] * mArray[4] - mArray[1] * mArray[3];

		TReal det = mArray[0] * inverse.mArray[0] +
					mArray[1] * inverse.mArray[3] +
					mArray[2] * inverse.mArray[6];

		if ( std::fabs( det ) > epsilon )
		{
			TReal invDet = 1 / det;
			for ( int i = 0; i < SIZE; ++i ) inverse.mArray[i] *= invDet;
			return inverse;
		}
		return ZERO;
	}
	//---------------------------------------

	//---------------------------------------
	// Adjoint
	M3_TMPL M3 M3::Adjoint() const
	{
		return M3
		(
			mArray[4] * mArray[8] - mArray[5] * mArray[7],
			mArray[2] * mArray[7] - mArray[1] * mArray[8],
			mArray[1] * mArray[5] - mArray[2] * mArray[4],
			mArray[5] * mArray[6] - mArray[3] * mArray[8],
			mArray[0] * mArray[8] - mArray[2] * mArray[6],
			mArray[2] * mArray[3] - mArray[0] * mArray[5],
			mArray[3] * mArray[7] - mArray[4] * mArray[6],
			mArray[1] * mArray[6] - mArray[0] * mArray[7],
			mArray[0] * mArray[4] - mArray[1] * mArray[3]
		);
	}
	//---------------------------------------

	//---------------------------------------
	// Determinant
	//---------------------------------------
	M3_TMPL TReal M3::Determinant() const
	{
		TReal co00 = mArray[4] * mArray[8] - mArray[5] * mArray[7];
		TReal co10 = mArray[5] * mArray[6] - mArray[5] * mArray[8];
		TReal co20 = mArray[3] * mArray[7] - mArray[5] * mArray[6];
		return mArray[0] * co00 + mArray[1] * co10 + mArray[2] * co20;
	}
	//---------------------------------------


	//---------------------------------------
	typedef Matrix3< float  > Matrix3f;
	typedef Matrix3< double > Matrix3d;
}