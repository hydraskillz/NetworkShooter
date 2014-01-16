#include "App.h"
#include "InputManager.h"

namespace mage
{

	//---------------------------------------
	static SDL_Surface* gDisplaySurf;
	static bool gIsRunning;

	static UpdateFn gUpdateFn;
	static RenderFn gRenderFn;
	static InputFn gInputFn;

	InputManager gInputManager;
	//---------------------------------------


	//---------------------------------------
	bool InitApp( const char* title, int w, int h )
	{
		// Initialize SDL
		if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
			return false;
		// Center the window
		SDL_putenv( "SDL_VIDEO_CENTERED=center" );
		// Set the video flags
		Uint32 flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF;
		// Create the window
		gDisplaySurf = SDL_SetVideoMode( w, h, 32, flags );
		if ( !gDisplaySurf )
			return false;
		SDL_WM_SetCaption( title, 0 );

		// Default callbacks
		RegisterUpdateFn( DefaultUpdateFn );
		RegisterRenderFn( DefaultRenderFn );
		RegisterInputFn( DefaultInputFn );

		// Load input bindings
		gInputManager.LoadKeyCodes( "KeyCodes.xml" );
		gInputManager.LoadInputBindings( "InputBindings.xml" );

		return true;
	}
	//---------------------------------------
	void Run()
	{
		gIsRunning = true;
		const float dt = 1.0f / 60.0f;
		SDL_Event sdlEvent;

		while ( gIsRunning )
		{
			while ( SDL_PollEvent( &sdlEvent ) )
			{
				if ( !gInputFn( sdlEvent ) )
				{
					gIsRunning = false;
					break;
				}
			}

			gInputManager.OnUpdate();

			gUpdateFn( dt );
			gRenderFn();

			SDL_GL_SwapBuffers();
		}

		SDL_FreeSurface( gDisplaySurf );
		SDL_Quit();
	}
	//---------------------------------------
	void AppExit()
	{
		gIsRunning = false;
	}
	//---------------------------------------
	bool DefaultInputFn( SDL_Event& sdlEvent )
	{
		switch ( sdlEvent.type )
		{
		case SDL_KEYDOWN:
			{
				if ( sdlEvent.key.keysym.sym == SDLK_ESCAPE )
				{
					return false;
					break;
				}
				else
				{
					gInputManager.SetKeyDown( sdlEvent.key.keysym.sym );
				}
			}
			break;
		case SDL_KEYUP:
			{
				gInputManager.SetKeyUp( sdlEvent.key.keysym.sym );
			}
			break;
		case SDL_QUIT:
			return false;
			break;
		}
		return true;
	}
	//---------------------------------------
	void DefaultUpdateFn( float dt )
	{
	}
	//---------------------------------------
	void DefaultRenderFn()
	{
	}
	//---------------------------------------
	void RegisterUpdateFn( UpdateFn fn )
	{
		gUpdateFn = fn;
	}
	//---------------------------------------
	void RegisterRenderFn( RenderFn fn )
	{
		gRenderFn = fn;
	}
	//---------------------------------------
	void RegisterInputFn( InputFn fn )
	{
		gInputFn = fn;
	}
	//---------------------------------------

}