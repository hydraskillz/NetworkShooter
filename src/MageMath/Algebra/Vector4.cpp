#include "Vector4.h"

namespace mage
{

	const Vec4f Vec4f::ZERO;
	const Vec4f Vec4f::ONE( 1.0f );
	const Vec4f Vec4f::UNIT_X( 1.0f, 0.0f, 0.0f, 0.0f );
	const Vec4f Vec4f::UNIT_Y( 0.0f, 1.0f, 0.0f, 0.0f );
	const Vec4f Vec4f::UNIT_Z( 0.0f, 0.0f, 1.0f, 0.0f );
	const Vec4f Vec4f::UNIT_W( 0.0f, 0.0f, 0.0f, 1.0f );

	const Vec4d Vec4d::ZERO;
	const Vec4d Vec4d::ONE( 1.0 );
	const Vec4d Vec4d::UNIT_X( 1.0, 0.0, 0.0, 0.0 );
	const Vec4d Vec4d::UNIT_Y( 0.0, 1.0, 0.0, 0.0 );
	const Vec4d Vec4d::UNIT_Z( 0.0, 0.0, 1.0, 0.0 );
	const Vec4d Vec4d::UNIT_W( 0.0, 0.0, 0.0, 1.0 );

}