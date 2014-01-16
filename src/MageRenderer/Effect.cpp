#include "RendererLib.h"

#include <glew.h>

using namespace mage;

//---------------------------------------
Effect::Effect()
	: mProgramId( 0 )
{}
//---------------------------------------
Effect::Effect( const Shader& vs, const Shader& fs )
{
	SetShaders( vs, fs );
}
//---------------------------------------
void Effect::SetShaders( const Shader& vs, const Shader& fs )
{
	GLint success = 0;
	GLchar errorlog[1024] = { 0 };

	// Create the program
	mProgramId = glCreateProgram();

	// Attach shaders
	glAttachShader( mProgramId, vs.GetShaderId() );
	glAttachShader( mProgramId, fs.GetShaderId() );

	// Link program
	glLinkProgram( mProgramId );

	// Check for errors
	glGetProgramiv( mProgramId, GL_LINK_STATUS, &success );
	if ( !success )
	{
		glGetProgramInfoLog( mProgramId, sizeof( errorlog ), 0, errorlog );
		assertion( false, "Error linking shader program. See stderr for more info.\n" ); 
	}

	// Validate program
	glValidateProgram( mProgramId );

	// Check for errors
	glGetProgramiv( mProgramId, GL_VALIDATE_STATUS, &success );
	if ( !success )
	{
		glGetProgramInfoLog( mProgramId, sizeof( errorlog ), 0, errorlog );
		assertion( false, "Invalid shader program. See stderr for more info.\n" ); 
	}
}
//---------------------------------------
Effect::~Effect()
{
	if ( mProgramId )
	{
		glDeleteProgram( mProgramId );
		mProgramId = 0;
	}
}
//---------------------------------------
void Effect::Apply() const
{
	glUseProgram( mProgramId );

	for ( auto itr = mUniforms.begin(); itr != mUniforms.end(); ++itr )
	{
		(*itr)->Apply();
	}
}
//---------------------------------------
int Effect::GetUniformLocation( const char* name ) const
{
	GLint location = glGetUniformLocation( mProgramId, name );

	// Warn bad name
	if ( location == INVALID_LOCATION )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: Unable to get the location of uniform '%s' in program '%s'.\n", name, "(todo names blah blah)" );
	}

	return location;
}
//---------------------------------------
int Effect::GetAttributeLocation( const char* name ) const
{
	int loc = glGetAttribLocation( mProgramId, name );
	// Warn bad name
	/*if ( loc == INVALID_LOCATION )
	{
		ConsolePrintf( CONSOLE_WARNING, "Warning: Unable to get the location of attrib '%s' in program '%s'.\n", name, "(todo names blah blah)" );
	}*/
	return loc;
}
//---------------------------------------
void Effect::AddUniform( Uniform* uniform )
{
	mUniforms.push_back( uniform );
}
//---------------------------------------