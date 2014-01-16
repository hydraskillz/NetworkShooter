/*
 * Author      : Matthew Johnson
 * Date        : 15/Aug/2013
 * Description :
 *   
 */
 
#pragma once

#include <SDL.h>

namespace mage
{

	typedef void(*UpdateFn)( float );
	typedef void(*RenderFn)( void );
	typedef bool(*InputFn)( SDL_Event& sdlEvent );


	// Initialize the app - do this before calling Run()
	bool InitApp( const char* title, int w, int h );
	// Run the app. Use AppExit() to exit or return false from InputFn
	void Run();
	// Exits the app (or you could call exit(0))
	void AppExit();

	bool DefaultInputFn( SDL_Event& sdlEvent );
	void DefaultUpdateFn( float dt );
	void DefaultRenderFn();
	
	// UpdateFn is passed a fixed timestep
	void RegisterUpdateFn( UpdateFn fn );
	// RenderFn is call after UpdateFn
	void RegisterRenderFn( RenderFn fn );
	// InputFn returns false to exit app, true to continue
	void RegisterInputFn( InputFn fn );

}