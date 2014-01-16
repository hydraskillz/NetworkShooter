#include <MageMath.h>
#include <MageCore.h>
#include <MageRenderer.h>
#include <SDL.h>

using namespace mage;

// Pass through shader
static const char* vs_pass =
	STRINGIFY(
	#version 130											\n
	in vec2 aPosition;										\n
	in int aPickingID;										\n
	flat out int vPickingID;								\n
	uniform mat4 uView;										\n
	void main()												\n
	{														\n
		vPickingID = aPickingID;							\n
		gl_Position = uView * vec4( aPosition, 0, 1 );		\n
	}
);

// Write picking ID
static const char* fs_picking =
	STRINGIFY(
	#version 130											\n
	flat in int vPickingID;									\n
	out vec4 fFragColor;									\n															\n
	void main()												\n
	{														\n
		fFragColor = vec4( vPickingID, 0, 0, 1 );		    \n
	}
);


Effect gPickingEffect;
Surface* gPickingSurf;
void Picking_Init()
{
	Shader pass( Shader::ST_VERTEX, vs_pass );
	Shader pick( Shader::ST_FRAGMENT, fs_picking );
	gPickingEffect.SetShaders( pass, pick );

	gPickingSurf = new Surface( 1280, 720, false, false );
}

void Picking_Begin()
{
	SetActiveSurface( gPickingSurf );
	SetActiveEffect( &gPickingEffect );
}


int main( int argc, char** argv )
{
	bool mFullscreen = false;
	SDL_Surface* mDisplaySurface = NULL;
	int bufferWidth = 256;
	int bufferHeight = 256;
	int mWindowWidth = 1280;
	int mWindowHeight = 720;
	float clearColor[] = { 0.25f, 0.5f, 0.25f };
	Texture* testTexture;
	Texture* texture1;
	Surface* sceneTarget;
	Surface* pass0;

	// Initialize SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
	{
		return 1;
	}

	if ( !Font::InitFont() )
	{
		return 1;
	}

	// Center the window
	SDL_putenv( "SDL_VIDEO_CENTERED=center" );
	// Set the video flags
	Uint32 flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE;
	// Fullscreen or not
	if ( mFullscreen )
	{
		flags |= SDL_FULLSCREEN;
	}
	// Create the window
	mDisplaySurface = SDL_SetVideoMode( mWindowWidth, mWindowHeight, 32, flags );

	if ( !mDisplaySurface )
	{
		return 1;
	}

	if ( !CreateRenderer() )
	{
		printf( "Failed to create renderer!\n" );
	}

	testTexture = new Texture( "../data/OrientationTest.png" );
	testTexture->Load();

	texture1 = new Texture( "../data/Wanderer.png" );
	texture1->Load();

	sceneTarget = new Surface( mWindowWidth, mWindowHeight, true, true, true );
	sceneTarget->ClearColor = Color::BLACK;

	int width = bufferWidth, height = bufferHeight;
	pass0 = new Surface( mWindowWidth, mWindowHeight, false, true );
	pass0->ClearColor = Color::TRANSPARENCY;

	SetViewport( 0, 0, mWindowWidth, mWindowHeight );
	SetOrthoView( 0, mWindowWidth, mWindowHeight, 0, 0.0, 1.0 );
	
	BloomEffect bloomEffectClass;
	bloomEffectClass.Initialize( mWindowWidth / 2, mWindowHeight / 2 );

	Picking_Init();

	bool mRunning = true;
	SDL_Event sdlEvent;
	while ( mRunning )
	{
		while ( SDL_PollEvent( &sdlEvent ) )
		{
			switch ( sdlEvent.type )
			{
			case SDL_KEYDOWN:
				{
					if ( sdlEvent.key.keysym.sym == SDLK_ESCAPE )
					{
						mRunning = false;
						break;
					}
					break;
				}

			case SDL_VIDEORESIZE:   // Window size has changed
				// Resize SDL video mode
				mWindowWidth = sdlEvent.resize.w;
				mWindowHeight = sdlEvent.resize.h;
				if( !SDL_SetVideoMode( mWindowWidth, mWindowHeight, 32, flags) )
					fprintf(stderr, "WARNING: Video mode set failed: %s\n", SDL_GetError());

				// Resize OpenGL viewport
				SetViewport( 0, 0, mWindowWidth, mWindowHeight );

				break;

			case SDL_QUIT:
				mRunning = false;
				break;
			}
		}

		// basic scene pass
		//SetActiveSurface( pass0 );
		Picking_Begin();
		ClearScreen();

		LoadIdentity();
		static float angle = 0;
		angle += 1.4f;

		Translate2D( bufferWidth / 2, bufferHeight / 2 );
		Rotate2D( angle );
		SetPickingID( 32 );
		DrawRect( texture1, -28, -28, 56, 56 );
		LoadIdentity();
		Translate2D( bufferWidth / 4, bufferHeight / 2 );
		Rotate2D( angle );
		SetPickingID( 128 );
		DrawRect( -28, -28, 56, 56, Color::BLUE.bgra );
		LoadIdentity();

		SetActiveSurface( sceneTarget );
		ClearScreen();
		ClearActiveEffect();
		DrawRect( gPickingSurf->TextureColor, 0, 0, mWindowWidth, mWindowHeight, Color::WHITE, true );
		FlushRenderer();

		//bloomEffectClass.Draw( pass0, sceneTarget );
		/*
		SetActiveSurface( sceneTarget );
		ClearScreen();

		// Normal scene
		DrawRect( pass0->TextureColor, 0, 0, true );
		// Draw glow extract
		DrawRect( bloomEffectClass.mExtract->TextureColor, pass0->Width, 0, true );
		// Draw blur horizontal
		DrawRect( bloomEffectClass.mSuraceA->TextureColor, 2 * pass0->Width, 0, true );
		// Draw blur vertical
		DrawRect( bloomEffectClass.mSuraceB->TextureColor, 3 * pass0->Width, 0, true );

		// Put boxes around stuff
		DrawRectOutline( 0, 0, pass0->Width, pass0->Height, Color::WHITE.bgra, 1.0f );
		DrawRectOutline( pass0->Width, 0, bloomExtractSurf.Width, bloomExtractSurf.Height, Color::WHITE.bgra, 1.0f );
		DrawRectOutline( 2 * pass0->Width, 0, bloomExtractSurf.Width, bloomExtractSurf.Height, Color::WHITE.bgra, 1.0f );
		DrawRectOutline( 3 * pass0->Width, 0, bloomExtractSurf.Width, bloomExtractSurf.Height, Color::WHITE.bgra, 1.0f );

		FlushRenderer();*/
		SDL_GL_SwapBuffers();
	}

	DestroyRenderer();

	SDL_FreeSurface( mDisplaySurface );
	SDL_Quit();
	Font::TermFont();

	delete pass0;
	delete sceneTarget;
	delete testTexture;
	delete texture1;
}