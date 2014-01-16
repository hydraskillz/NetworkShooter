#include <MageMath.h>
#include <MageCore.h>

using namespace mage;

int main( int argc, char** argv )
{
	SetPathToData( "../data", 0 );


	// Xml Writer test
	XmlWriter writer( "xml/test.xml" );

	writer.BeginElement( "Test" );
	{
		writer.AddAttribute( "testAttr", "hello world" );
		writer.AddAttribute( "testAttr2", std::string( "hello world again" ) );
		writer.AddAttribute( "testBool", true );
		writer.AddAttribute( "testInt", -680 );
		writer.AddAttribute( "testUnsigned", 500 );
		writer.AddAttribute( "testFloat", Mathf::PI );
		writer.AddAttribute( "testDouble", Mathd::PI );
		writer.BeginElement( "subElem" );
		{
			writer.AddCData( "This is some CData!" );
		}
		writer.EndElement();

		// Vector floats
		writer.BeginElement( "VectorTests" );
		{
			writer.AddAttribute( "vec2f", Vec2f( 1.0f, 2.5f ) );
			writer.AddAttribute( "vec3f", Vec3f( 1.0f, 2.5f, 3.2f ) );
			writer.AddAttribute( "vec4f", Vec4f( 1.0f, 2.5f, 3.2f, 5.4f ) );
		}
		writer.EndElement();

		// Vector ints
		writer.BeginElement( "VectorTests" );
		{
			writer.AddAttribute( "vec2i", Vec2i( 1.0f, 2.5f ) );
			writer.AddAttribute( "vec3i", Vec3i( 1.0f, 2.5f, 3.2f ) );
			writer.AddAttribute( "vec4i", Vec4i( 1.0f, 2.5f, 3.2f, 5.4f ) );
		}
		writer.EndElement();

		// CSV attribute
		std::vector< int > vec;
		vec.push_back( 5 );
		vec.push_back( 34 );
		vec.push_back( 6 );
		vec.push_back( 98 );
		vec.push_back( 14 );
		std::vector< std::string > vec2;
		vec2.push_back( "dog" );
		vec2.push_back( "cat" );
		vec2.push_back( "bear" );
		vec2.push_back( "wolf" );
		vec2.push_back( "dragon" );
		writer.BeginElement( "CSV attr" );
		{
			writer.AddAttribute( "csv", vec );
			writer.AddAttribute( "csv", vec2 );
		}
		writer.EndElement();

		// Color
		writer.BeginElement( "Colors" );
		{
			writer.AddAttribute( "color1", Color::RED );
			writer.AddAttribute( "color2", Color::LIGHT_BLUE );
		}
		writer.EndElement();
	}
	writer.EndElement();

	return 0;
}