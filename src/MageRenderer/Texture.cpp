#include "RendererLib.h"

#include <SDL.h>
#include <SDL_image.h>

using namespace mage;

Texture::Texture( uint32 textureId, uint32 w, uint32 h )
	: mIsLoaded( true )
	, mFilename( 0 )
	, mId( textureId )
	, mWidth( w )
	, mHeight( h )
{}

Texture::Texture( const char* filename )
	: mIsLoaded( false )
	, mFilename( filename )
	, mId( 0 )
	, mWidth( 0 )
	, mHeight( 0 )
{}

Texture::Texture( SDL_Surface* surf )
	: mIsLoaded( true )
{
	LoadFromSurface( surf );
}

Texture::~Texture()
{
	FreeTexture( &mId );
	mId = 0;
}

bool Texture::Load()
{
	SDL_Surface* surf = IMG_Load( mFilename );

	if ( LoadFromSurface( surf ) )
	{
		SDL_FreeSurface( surf );

		return true;
	}
	return false;
}

bool Texture::LoadFromSurface( SDL_Surface* surf )
{
	if ( surf )
	{
		IRenderer::PixelFormat format;
		if ( surf->format->BytesPerPixel == 4 )
		{
			format = IRenderer::PF_RGBA;
			//if ( surf->format->Rmask == 0x000000ff )
			//	format = IRenderer::PF_RGBA;
			//else
			//	format = IRenderer::PF_BGRA;
		}
		else
		{
			format = IRenderer::PF_RGB;
			//if ( surf->format->Rmask == 0x000000ff )
			//	format = IRenderer::PF_RGB;
			//else
			//	format = IRenderer::PF_BGR;
		}

		mWidth = surf->w;
		mHeight = surf->h;

		CreateTexture( &mId, surf->pixels, mWidth, mHeight, format );

		return true;
	}
	return false;
}