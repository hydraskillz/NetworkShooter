#pragma once

#ifdef MAGE_USE_GLRENDERER
#	include "GLRenderer/GLRenderer.h"
#	define CreateRenderer()						\
		InitRenderer( new mage::GLRenderer() )
#	define IRenderCall( x ) { assertion( pRenderer != NULL, "You must call 'CreateRenderer()' first!!\n" ); pRenderer->x; }
#else
#	define NO_RENDERER_PRESENT "No renderer is present. All draw calls are nops."
#	if _MSC_VER
#		pragma message( NO_RENDERER_PRESENT )
#	else	// GCC
#		warning( NO_RENDERER_PRESENT )
#	endif
#	define CreateRenderer()						\
		InitRenderer( 0 )
#	define IRenderCall( x )
#endif

//#	define IRenderCall( x ) pRenderer->x

//class IRenderer;
#include "IRenderer.h"

namespace mage
{

	bool InitRenderer( IRenderer* renderer );
	void DestroyRenderer();
	void FlushRenderer();
	void ClearScreen();

	void DrawRect( float x, float y, float w, float h, const mage::Color& color );
	void DrawRectOutline( float x, float y, float w, float h, const mage::Color& color, float lineWidth=1.0f );
	void DrawRectOutlined( float x, float y, float w, float h, const mage::Color& color, float outlineWidth, const mage::Color& outlineColor );
	void DrawRect( Texture* texture, float x, float y, float w, float h, const mage::Color& color, bool yUp=false );
	void DrawRect( Texture* texture, float x, float y, float w, float h, const mage::RectF& fclip, const mage::Color& color, bool yUp=false );
	void DrawRect( Texture* texture, float x, float y, bool yUp=false );
	void DrawRect( Texture* texture, float x, float y, float w, float h, bool yUp=false );
	void DrawRect( Texture* texture, float x, float y, const mage::Color& color );
	void DrawRect( Texture* texture, float x, float y, const mage::RectI& clip, bool yUp=false );

	void DrawLine( const mage::Vec2f& A, const mage::Vec2f& B, float width, const mage::Color& color );
	void DrawLine( float x0, float y0, float x1, float y1, float width, const mage::Color& color );
	void DrawGradientLine( float x0, float y0, float x1, float y1, float width, const mage::Color& colorStart, const mage::Color& colorEnd );

	void DrawPoint( float x, float y, float pointSize, const mage::Color& color );
	void DrawPoints( float* points, unsigned int nPoints, float pointSize, const mage::Color& color );

	void DrawPolyLine( float* points, size_t nPoints, float width, unsigned int color, bool loop );
	void DrawPolygon( float* points, size_t nPoints, unsigned int color );
	void DrawPolygonOutlined( float* points, size_t nPoints, unsigned int color, float outlineWidth, unsigned int outlineColor );

	void DrawCircle( float cx, float cy, float r, const mage::Color& color );

	// Draw text (dynamic text)
	void DrawText( float x, float y, mage::Font* font, const mage::Color& color, const char* text );
	void DrawText( float x, float y, const mage::Color& color, const char* text );
	void DrawText( float x, float y, const char* text );
	// Draw text (dynamic text) with formating
	void DrawTextFormat( float x, float y, mage::Font* font, const mage::Color& color, const char* text, ... );
	void DrawTextFormat( float x, float y, const mage::Color& color, const char* text, ... );
	void DrawTextFormat( float x, float y, const char* text, ... );

	void SetViewMatrix( const float* view );

	void CreateTexture( IRenderer::TextureHandle* hTexture, void* pixels, unsigned int w, unsigned int h, IRenderer::PixelFormat format, bool linearFilter=true );
	void FreeTexture( IRenderer::TextureHandle* hTexture );

	void SetClearColor( float r, float g, float b, float a );
	void SetViewport( int x, int y, int w, int h );
	void SetOrthoView( float left, float right, float bottom, float top, float zNear, float zFar );

	void PushMatrix();
	void PopMatrix();
	void LoadIdentity();
	void LoadMatrix( float* m );

	void Translate( float x, float y, float z );
	void Rotate( float angleDeg, float x, float y, float z );
	void Scale( float x, float y, float z );
	void ScaleUniform( float scale );

	void Translate2D( const mage::Vec2f& trans );
	void Translate2D( float x, float y );
	void Rotate2D( float angleDeg );
	void Scale2D( float x, float y );
	void ScaleUniform2D( float scale );

	// Deprecated DO NOT USE - use the flag in the draw calls to invert texture coords (or just specify them yourself)
	void SetTextureYUp( bool up );
	
	// Will cause FlushRenderer() if effect is different from the current effect
	void SetActiveEffect( Effect* effect );
	// Will cause FlushRenderer() if active effect is not the default
	void ClearActiveEffect();
	void BindTexture( Texture* texture, int channel );
	// Will cause FlushRenderer() if the surface is different from the active surface
	void SetActiveSurface( Surface* surface );

	// Set the ID to write to the picking buffer
	void SetPickingID( int id );
}