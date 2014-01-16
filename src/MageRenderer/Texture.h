#pragma once

struct SDL_Surface;

namespace mage
{

	class Texture
	{
	public:
		Texture( uint32 textureId, uint32 w, uint32 h );
		Texture( const char* filename );
		Texture( SDL_Surface* surf );
		~Texture();

		bool Load();
		bool LoadFromSurface( SDL_Surface* surf );

		inline unsigned int GetWidth() const { return mWidth; }
		inline unsigned int GetHeight() const { return mHeight; }
		inline unsigned int GetId() const { return mId; }
	
	private:
		unsigned int mWidth, mHeight;
		bool mIsLoaded;
		const char* mFilename;
		unsigned int mId;
	};

}