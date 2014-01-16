#pragma once

class IRenderer
{
public:
	virtual ~IRenderer() {}
	enum RenderMode
	{
		None,			// No primitives should be rendered
		Triangles,
		Lines,
		Points
	};

	struct Vertex2D
	{
		float x, y;
		float u, v;
		unsigned char rgba[4];
		int pickingID;
		float pointsize;
		float padding;		// Make vertex multiple of 32
	};

	struct VertexList
	{
		static const size_t MAX_VLIST_SIZE = 32;
		Vertex2D Verts[ MAX_VLIST_SIZE ];
		size_t NumVerts;
	};

	typedef unsigned int TextureHandle;
	enum PixelFormat
	{
		PF_RGB,
		PF_RGBA,
		PF_BGR,
		PF_BGRA
	};

	virtual void CreateTexture( IRenderer::TextureHandle* hTexture, void* pixels, unsigned int w, unsigned int h, IRenderer::PixelFormat format, bool linearFilter=true ) = 0;
	virtual void FreeTexture( IRenderer::TextureHandle* hTexture ) = 0;

	// This call is a draw request. Drawing may be delayed until FlushRenderer() is called.
	virtual void RenderVerticies( RenderMode mode, IRenderer::TextureHandle texture, const VertexList& verts ) = 0;
	// Called when drawing must occur.
	virtual void FlushRenderer() = 0;
	// Set the effect program to be used when drawing.
	virtual void SetActiveEffect( mage::Effect* effect ) = 0;
	// Clear the active effect program and use the default one
	virtual void ClearActiveEffect() = 0;
	// Set the current view
	virtual void SetViewMatrix( const float* view ) = 0;
	// Clear the screen to the clear color
	virtual void ClearScreen() = 0;
	// Set the clear color
	virtual void SetClearColor( float r, float g, float b, float a ) = 0;
	virtual void SetViewport( int x, int y, int w, int h ) = 0;
	// Bind a texture to a specific channel
	virtual void BindTexture( IRenderer::TextureHandle hTexture, int channel ) = 0;
};