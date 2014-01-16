#include "RendererLib.h"

#include "GLRenderer.h"
#include "glew.h"

#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )

using namespace mage;

#define USE_GL33 1

static const char* vs_basic =
	"#version 130\n"
	"in vec2 aPosition;\n"
	"in vec4 aColor;\n"
	"in vec2 aTexCoord0;\n"
	"in float aPointsize;\n"
	"out vec4 vColor;\n"
	"out vec2 vTexCoord0;\n"
	"uniform mat4 uView;\n"
	"void main() {\n"
	"	vColor = aColor;\n"
	"	vTexCoord0 = aTexCoord0;\n"
	"	gl_Position = uView * vec4( aPosition, 0, 1 );\n"
	"	gl_PointSize = aPointsize;\n"
	"}";

static const char* fs_basic =
	"#version 130\n"
	"in vec4 vColor;\n"
	"in vec2 vTexCoord0;\n"
	"uniform sampler2D uTextureSampler0;\n"
	"uniform bool uUseTex2D;\n"
	"out vec4 fFragColor;\n"
	"void main() {\n"
	"	if ( uUseTex2D )\n"
	"	{\n"
	"		vec4 texel = texture2D( uTextureSampler0, vTexCoord0 ) * vColor;\n"
	"		fFragColor = texel;\n"
	"	}\n"
	"	else\n"
	"		fFragColor = vec4( vColor.rgb, vColor.a );\n"
	"}";

// Base effect - uses vs_basic and fs_basic
static Effect* gBasicEffect;
static Shader* basicVS;
static Shader* basicFS;

static Uniform1i* gDefaultTextureSampler;
static Uniform1i* gUseTexture;

GLRenderer::GLRenderer()
{
	GLenum glewErr = glewInit();
	if ( glewErr != GLEW_OK )
	{
		assertion( false, "glewInit failed!\n%s\n",  glewGetErrorString( glewErr ) );
	}
	
	// Enable transparency
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Enable point size in shader
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

	mCurrentRenderMode = IRenderer::None;
	mCurrentTexture = 0;
	mActiveTexture = 0;
	mCurrentBufferCount = 0;

	// Create basic effect for simple rendering
	basicVS = new Shader( Shader::ST_VERTEX, vs_basic );
	basicFS = new Shader( Shader::ST_FRAGMENT, fs_basic );
	gBasicEffect = new Effect( *basicVS, *basicFS );

	gDefaultTextureSampler = new Uniform1i( gBasicEffect->GetUniformLocation( "uTextureSampler0" ) );
	gUseTexture = new Uniform1i( gBasicEffect->GetUniformLocation( "uUseTex2D" ) );

	gBasicEffect->AddUniform( gDefaultTextureSampler );
	gBasicEffect->AddUniform( gUseTexture );

	mActiveEffect = gBasicEffect;

	//mCurrentProgram = new ShaderProgram( vs, fs );
	//memset( mView, 0, sizeof( float ) * 16 );
}

GLRenderer::~GLRenderer()
{
	std::for_each( mTextures.begin(), mTextures.end(), [&]( IRenderer::TextureHandle& id )
	{
		glDeleteTextures( 1, &id );
	});
}

void GLRenderer::CopyVertexListToBuffer( const VertexList& verts )
{
	// Flush if buffer is full
	if ( mCurrentBufferCount + verts.NumVerts >= MAX_VERTEX_BATCH )
	{
		FlushRenderer();
	}
	// Copy verts to buffer
	memcpy( mVertexBuffer + mCurrentBufferCount, verts.Verts, sizeof( Vertex2D ) * verts.NumVerts );
	mCurrentBufferCount += verts.NumVerts;
}

void GLRenderer::RenderVerticies( RenderMode mode, IRenderer::TextureHandle texture, const VertexList& verts )
{
	// Flush on required state change
	if ( mode != mCurrentRenderMode || texture != mCurrentTexture )
	{
		FlushRenderer();

		mCurrentTexture = texture;
		mCurrentRenderMode = mode;
	}
	// Copy verts to be rendered
	CopyVertexListToBuffer( verts );
}

void GLRenderer::SetViewMatrix( const float* view )
{
#if USE_GL33
	memcpy( mView, view, sizeof( float ) * 16 );
#else
	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( view );
	glMatrixMode( GL_MODELVIEW );
#endif
}

void GLRenderer::ClearScreen()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void GLRenderer::SetClearColor( float r, float g, float b, float a )
{
	glClearColor( r, g, b, a );
}

void GLRenderer::SetViewport( int x, int y, int w, int h )
{
	glViewport( x, y, w, h );
}

void GLRenderer::CreateTexture( TextureHandle* hTexture, void* pixels, unsigned int w, unsigned int h, IRenderer::PixelFormat format, bool linearFilter )
{
	static GLenum GLFormat[] =
	{
		GL_RGB,
		GL_RGBA,
		GL_BGR_EXT,
		GL_BGRA_EXT
	};

	GLenum filter = linearFilter ? GL_LINEAR : GL_NEAREST;

	glGenTextures( 1, hTexture );

	glBindTexture( GL_TEXTURE_2D, *hTexture );
//	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexImage2D( GL_TEXTURE_2D, 0, GLFormat[ format ], w, h, 0, GLFormat[ format ], GL_UNSIGNED_BYTE, pixels );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	
	glBindTexture( GL_TEXTURE_2D, 0 );

	mTextures.push_back( *hTexture );
}

void GLRenderer::FreeTexture( TextureHandle* hTexture )
{
	int sizeBeforeRemove = mTextures.size();
	mTextures.remove( *hTexture );
	int deltaSize = sizeBeforeRemove - mTextures.size();
	if ( deltaSize )
		glDeleteTextures( 1, hTexture );
}

void GLRenderer::FlushRenderer()
{
	static GLenum GLRenderModes[] =
	{
		0,
		GL_TRIANGLES,
		GL_LINES,
		GL_POINTS
	};

	if ( mCurrentRenderMode != IRenderer::None )
	{
#if USE_GL33
		
		// Set active texture
		if ( mCurrentTexture != mActiveTexture )
		{
			mActiveTexture = mCurrentTexture;
		}
		// Apply active texture if using default shader
		if ( mActiveEffect == gBasicEffect )
		{
			BindTexture( mActiveTexture, 0 );
			gDefaultTextureSampler->SetValue( 0 );
			gUseTexture->SetValue( mActiveTexture != 0 );

			//glActiveTexture( GL_TEXTURE0 );
			//glBindTexture( GL_TEXTURE_2D, mActiveTexture );
			//glUniform1i( mActiveEffect->GetUniformLocation( "uTextureSampler0" ), 0 );
			//glUniform1i( mActiveEffect->GetUniformLocation( "uUseTex2D" ), mActiveTexture != 0 );
		}

		// Use active effect program
		mActiveEffect->Apply();

		int viewLoc = mActiveEffect->GetUniformLocation( "uView" );
		glUniformMatrix4fv( viewLoc, 1, GL_FALSE, mView );


		GLuint aPosLoc = mActiveEffect->GetAttributeLocation( "aPosition" );
		GLuint aColLoc = mActiveEffect->GetAttributeLocation( "aColor" );
		GLuint aTexLoc = mActiveEffect->GetAttributeLocation( "aTexCoord0" );
		GLuint aPickId = mActiveEffect->GetAttributeLocation( "aPickingID" );
		GLuint aPointsize = mActiveEffect->GetAttributeLocation( "aPointsize" );

		glEnableVertexAttribArray( aPosLoc );
		glEnableVertexAttribArray( aColLoc );
		glEnableVertexAttribArray( aTexLoc );
		if ( aPickId != Effect::INVALID_LOCATION )
			glEnableVertexAttribArray( aPickId );
		if ( aPointsize != Effect::INVALID_LOCATION )
			glEnableVertexAttribArray( aPointsize );


		glVertexAttribPointer( aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), mVertexBuffer );
		glVertexAttribPointer( aColLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( Vertex2D ), &mVertexBuffer[0].rgba[0] );
		glVertexAttribPointer( aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), &mVertexBuffer[0].u );
		if ( aPickId != Effect::INVALID_LOCATION )
			glVertexAttribPointer( aPickId, 1, GL_INT, GL_FALSE, sizeof( Vertex2D ), &mVertexBuffer[0].pickingID );
		if ( aPointsize != Effect::INVALID_LOCATION )
			glVertexAttribPointer( aPointsize, 1, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), &mVertexBuffer[0].pointsize );
		
		glDrawArrays( GLRenderModes[ mCurrentRenderMode ], 0, mCurrentBufferCount );

		glDisableVertexAttribArray( aPosLoc );
		glDisableVertexAttribArray( aColLoc );
		glDisableVertexAttribArray( aTexLoc );
		if ( aPickId != Effect::INVALID_LOCATION )
			glDisableVertexAttribArray( aPickId );
		if ( aPointsize != Effect::INVALID_LOCATION )
			glDisableVertexAttribArray( aPointsize );
#else
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );
		if ( mCurrentTexture )
		{
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glEnable( GL_TEXTURE_2D );
			if ( mCurrentTexture != mActiveTexture )
			{
				glBindTexture( GL_TEXTURE_2D, mCurrentTexture );
				mActiveTexture = mCurrentTexture;
			}
		}

		glVertexPointer( 2, GL_FLOAT,         sizeof( Vertex2D ), &mVertexBuffer[0].x );
		glColorPointer(  4, GL_UNSIGNED_BYTE, sizeof( Vertex2D ), &mVertexBuffer[0].rgba[0] );
		glTexCoordPointer( 2, GL_FLOAT,       sizeof( Vertex2D ), &mVertexBuffer[0].u );

		glDrawArrays( GLRenderModes[ mCurrentRenderMode ], 0, mCurrentBufferCount );

		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_COLOR_ARRAY );
		if ( mCurrentTexture )
		{
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			glDisable( GL_TEXTURE_2D );
			//glBindTexture( GL_TEXTURE_2D, 0 );
		}
#endif
	}

	mCurrentBufferCount = 0;
}

void GLRenderer::SetActiveEffect( Effect* effect )
{
	// Flush if switching to a new effect
	if ( mActiveEffect != effect )
		FlushRenderer();

	mActiveEffect = effect;
}

void GLRenderer::ClearActiveEffect()
{
	// Flush if we are switching back to default
	if ( mActiveEffect != gBasicEffect )
		FlushRenderer();

	mActiveEffect = gBasicEffect;
}

void GLRenderer::BindTexture( IRenderer::TextureHandle hTexture, int channel )
{
	glActiveTexture( GL_TEXTURE0 + channel );
	glBindTexture( GL_TEXTURE_2D, hTexture );
}