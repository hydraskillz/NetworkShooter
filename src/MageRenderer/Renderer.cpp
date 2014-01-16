#include "RendererLib.h"

//using namespace mage;
namespace mage
{

static IRenderer* pRenderer;
static std::stack< Matrix4f > gMatrixStack;
static Matrix4f gCurrentMatrix;
static bool gYUp = false;
static int gPickingID;
static Font* gDefaultFont;

static Surface* gActiveSurface;

bool InitRenderer( IRenderer* renderer )
{
	pRenderer = renderer;
	if ( !pRenderer )
		return false;

	if ( !Font::InitFont() )
		return false;

	gDefaultFont = new Font( "C:/windows/fonts/arial.ttf", 14 );

	return true;
}

void DestroyRenderer()
{
	if ( pRenderer )
	{
		delete pRenderer;
		pRenderer = NULL;
	}
}

void SetTextureYUp( bool up )
{
	gYUp = up;
}

void FlushRenderer()
{
	IRenderCall( FlushRenderer() );
}

void DrawRect( float x, float y, float w, float h, const mage::Color& color )
{
	DrawRect( 0, x, y, w, h, color, false );
}

void DrawRectOutline( float x, float y, float w, float h, const mage::Color& color, float lineWidth )
{
	DrawLine( x, y, x + w, y, lineWidth, color );
	DrawLine( x + w, y, x + w, y + h, lineWidth, color );
	DrawLine( x + w, y + h, x, y + h, lineWidth, color );
	DrawLine( x, y + h, x, y, lineWidth, color );
	/*
	IRenderer::VertexList verts;

	verts.Verts[ 0 ].x = x;
	verts.Verts[ 0 ].y = y;

	verts.Verts[ 1 ].x = x + w;
	verts.Verts[ 1 ].y = y;

	verts.Verts[ 2 ].x = x + w;
	verts.Verts[ 2 ].y = y + h;

	verts.Verts[ 3 ].x = x;
	verts.Verts[ 3 ].y = y + h;

	verts.NumVerts = 4;

	pRenderer->RenderVerticies( IRenderer::Lines, 0, verts );
	*/
}

void DrawRectOutlined( float x, float y, float w, float h, const mage::Color& color, float outlineWidth, const mage::Color& outlineColor )
{
	DrawRect( x, y, w, h, color );
	DrawRectOutline( x, y, w, h, outlineColor, outlineWidth );
}

void GetColorFormUInt( unsigned char rgba[4], unsigned int color )
{
	rgba[3] = (unsigned char) ( ( color              ) >> 24 );
	rgba[0] = (unsigned char) ( ( color & 0x00FF0000 ) >> 16 );
	rgba[1] = (unsigned char) ( ( color & 0x0000FF00 ) >> 8  );
	rgba[2] = (unsigned char) ( ( color & 0x000000FF )       );
}

void DrawRect( Texture* texture, float x, float y, float w, float h, bool yUp )
{
	DrawRect( texture, x, y, w, h, Color::WHITE, yUp );
}

void DrawRect( Texture* texture, float x, float y, bool yUp )
{
	DrawRect( texture, x, y, (float) texture->GetWidth(), (float) texture->GetHeight(), 0xFFFFFFFF, yUp );
}

void DrawRect( Texture* texture, float x, float y, const mage::Color& color )
{
	DrawRect( texture, x, y, (float) texture->GetWidth(), (float) texture->GetHeight(), color );
}

void DrawRect( Texture* texture, float x, float y, const mage::RectI& clip, bool yUp )
{
	mage::RectF fclip;
	fclip.Left   = clip.Left   / (float) texture->GetWidth();
	fclip.Right  = clip.Right  / (float) texture->GetWidth();
	fclip.Top    = clip.Top    / (float) texture->GetHeight();
	fclip.Bottom = clip.Bottom / (float) texture->GetHeight();

	DrawRect( texture, x, y, (float) texture->GetWidth(), (float) texture->GetHeight(), fclip, 0xFFFFFFFF, yUp );
}

void DrawRect( Texture* texture, float x, float y, float w, float h, const mage::RectF& fclip, const mage::Color& color, bool yUp )
{
	IRenderer::VertexList verts;

	unsigned char rgba[4];
	GetColorFormUInt( rgba, color.bgra );

	Vec3f v[6];
	
	v[0].Set( x, y + h, 0 );
	v[0] = gCurrentMatrix * v[0];

	v[1].Set( x, y, 0 );
	v[1] = gCurrentMatrix * v[1];

	v[2].Set( x + w, y, 0 );
	v[2] = gCurrentMatrix * v[2];

	v[3].Set( x, y + h, 0 );
	v[3] = gCurrentMatrix * v[3];

	v[4].Set( x + w, y + h, 0 );
	v[4] = gCurrentMatrix * v[4];

	v[5].Set( x + w, y, 0 );
	v[5] = gCurrentMatrix * v[5];

	for ( int i = 0; i < 6; ++i )
		memcpy( &verts.Verts[ i ], &v[ i ], 2 * sizeof( float ) );
	//verts.Verts[ 0 ].x = x;
	//verts.Verts[ 0 ].y = y + h;
	verts.Verts[ 0 ].u = fclip.Left;
	verts.Verts[ 0 ].v = yUp ? fclip.Top : fclip.Bottom;
	memcpy( verts.Verts[ 0 ].rgba, rgba, 4 );

	//verts.Verts[ 1 ].x = x;
	//verts.Verts[ 1 ].y = y;
	verts.Verts[ 1 ].u = fclip.Left;
	verts.Verts[ 1 ].v = yUp ? fclip.Bottom : fclip.Top;
	memcpy( verts.Verts[ 1 ].rgba, rgba, 4 );

	//verts.Verts[ 2 ].x = x + w;
	//verts.Verts[ 2 ].y = y;
	verts.Verts[ 2 ].u = fclip.Right;
	verts.Verts[ 2 ].v = yUp ? fclip.Bottom : fclip.Top;
	memcpy( verts.Verts[ 2 ].rgba, rgba, 4 );

	//verts.Verts[ 3 ].x = x;
	//verts.Verts[ 3 ].y = y + h;
	verts.Verts[ 3 ].u = fclip.Left;
	verts.Verts[ 3 ].v = yUp ? fclip.Top : fclip.Bottom;
	memcpy( verts.Verts[ 3 ].rgba, rgba, 4 );

	//verts.Verts[ 4 ].x = x + w;
	//verts.Verts[ 4 ].y = y + h;
	verts.Verts[ 4 ].u = fclip.Right;
	verts.Verts[ 4 ].v = yUp ? fclip.Top : fclip.Bottom;
	memcpy( verts.Verts[ 4 ].rgba, rgba, 4 );

	//verts.Verts[ 5 ].x = x + w;
	//verts.Verts[ 5 ].y = y;
	verts.Verts[ 5 ].u = fclip.Right;
	verts.Verts[ 5 ].v = yUp ? fclip.Bottom : fclip.Top;
	memcpy( verts.Verts[ 5 ].rgba, rgba, 4 );

	verts.NumVerts = 6;

	IRenderer::TextureHandle hTexture = 0;
	if ( texture )
		hTexture = texture->GetId();

	IRenderCall( RenderVerticies( IRenderer::Triangles, hTexture, verts ) );
}

void DrawRect( Texture* texture, float x, float y, float w, float h, const mage::Color& color, bool yUp )
{
	DrawRect( texture, x, y, w, h, mage::RectF( 0, 0, 1, 1 ), color, yUp );
}

void DrawLine( const mage::Vec2f& A, const mage::Vec2f& B, float width, const mage::Color& color )
{
	DrawLine( A.x, A.y, B.x, B.y, width, color.bgra );
}

void DrawLine( float x0, float y0, float x1, float y1, float width, const mage::Color& color )
{
	DrawGradientLine( x0, y0, x1, y1, width, color, color );
}

void DrawGradientLine( float x0, float y0, float x1, float y1, float width, const mage::Color& colorStart, const mage::Color& colorEnd )
{
	IRenderer::VertexList verts;
#ifdef TRIANGLE_LINES
	const float halfWidth = width / 2.0f;
	Vec3f p1( x0, y0, 0 );
	Vec3f p2( x1, y1, 0 );
	Vec3f dir = p2 - p1;

	dir.Normalize();

	Vec3f vperp( -dir.y, dir.x, 0 );

	Vec3f v[4];

	v[ 0 ] = gCurrentMatrix * ( p1 + halfWidth * vperp );
	v[ 1 ] = gCurrentMatrix * ( p1 - halfWidth * vperp );
	v[ 2 ] = gCurrentMatrix * ( p2 + halfWidth * vperp );
	v[ 3 ] = gCurrentMatrix * ( p2 - halfWidth * vperp );

	unsigned char rgbaBegin[4];
	unsigned char rgbaEnd[4];
	GetColorFormUInt( rgbaBegin, colorStart.bgra );
	GetColorFormUInt( rgbaEnd, colorEnd.bgra );

	verts.Verts[ 0 ].x = v[ 0 ].x;
	verts.Verts[ 0 ].y = v[ 0 ].y;
	memcpy( verts.Verts[ 0 ].rgba, rgbaBegin, 4 );

	verts.Verts[ 1 ].x = v[ 1 ].x;
	verts.Verts[ 1 ].y = v[ 1 ].y;
	memcpy( verts.Verts[ 1 ].rgba, rgbaBegin, 4 );

	verts.Verts[ 2 ].x = v[ 2 ].x;
	verts.Verts[ 2 ].y = v[ 2 ].y;
	memcpy( verts.Verts[ 2 ].rgba, rgbaEnd, 4 );

	verts.Verts[ 3 ].x = v[ 2 ].x;
	verts.Verts[ 3 ].y = v[ 2 ].y;
	memcpy( verts.Verts[ 3 ].rgba, rgbaEnd, 4 );

	verts.Verts[ 4 ].x = v[ 3 ].x;
	verts.Verts[ 4 ].y = v[ 3 ].y;
	memcpy( verts.Verts[ 4 ].rgba, rgbaEnd, 4 );

	verts.Verts[ 5 ].x = v[ 1 ].x;
	verts.Verts[ 5 ].y = v[ 1 ].y;
	memcpy( verts.Verts[ 5 ].rgba, rgbaBegin, 4 );

	verts.NumVerts = 6;

	IRenderCall( RenderVerticies( IRenderer::Triangles, 0, verts ) );
#else
	unsigned char rgbaBegin[4];
	unsigned char rgbaEnd[4];

	GetColorFormUInt( rgbaBegin, colorStart.bgra );
	GetColorFormUInt( rgbaEnd, colorEnd.bgra );

	Vec3f p1( x0, y0, 0 );
	Vec3f p2( x1, y1, 0 );
	Vec3f v[2];

	v[ 0 ] = gCurrentMatrix * p1;
	v[ 1 ] = gCurrentMatrix * p2;

	verts.Verts[ 0 ].x = v[0].x;
	verts.Verts[ 0 ].y = v[0].y;
	memcpy( verts.Verts[ 0 ].rgba, rgbaBegin, 4 );

	verts.Verts[ 1 ].x = v[1].x;
	verts.Verts[ 1 ].y = v[1].y;
	memcpy( verts.Verts[ 1 ].rgba, rgbaEnd, 4 );

	verts.NumVerts = 2;

	IRenderCall( RenderVerticies( IRenderer::Lines, 0, verts ) );
#endif
}

void DrawPoint( float x, float y, float pointSize, const mage::Color& color )
{
	IRenderer::VertexList verts;
	unsigned char rgba[4];
	GetColorFormUInt( rgba, color.bgra );

	Vec3f v( x, y, 0 );
	v = gCurrentMatrix * v;

	verts.Verts[ 0 ].x = v.x;
	verts.Verts[ 0 ].y = v.y;
	verts.Verts[ 0 ].pointsize = pointSize;
	memcpy( verts.Verts[ 0 ].rgba, rgba, 4 );

	verts.NumVerts = 1;

	IRenderCall( RenderVerticies( IRenderer::Points, 0, verts ) );
}

void DrawPoints( float* points, unsigned int nPoints, float pointSize, const mage::Color& color )
{
	IRenderer::VertexList verts;
	unsigned char rgba[4];
	GetColorFormUInt( rgba, color.bgra );

	const size_t N = nPoints * 2;
	for ( size_t i = 0, j = 0; i < N; i += 2, ++j )
	{
		Vec3f v( points[ i ], points[ i + 1 ], 0 );
		v = gCurrentMatrix * v;
		verts.Verts[ j ].x = v.x;
		verts.Verts[ j ].y = v.y;
		verts.Verts[ j ].pointsize = pointSize;
		memcpy( verts.Verts[ j ].rgba, rgba, 4 );
	}

	verts.NumVerts = nPoints;

	IRenderCall( RenderVerticies( IRenderer::Points, 0, verts ) );
}

void DrawPolyLine( float* points, size_t nPoints, float width, unsigned int color, bool loop )
{
	const size_t N = 2 * nPoints;
	for ( size_t i = 0; i < N; i+=2 )
	{
		if ( i + 2 < N )
		{
			DrawLine( points[i], points[i+1], points[i+2], points[i+3], width, color );
		}
	}

	// Draw edge between first and last point
	if ( loop && nPoints > 2 )
	{
		DrawLine( points[N-2], points[N-1], points[0], points[1], width, color );
	}
}

void DrawPolygon( float* points, size_t nPoints, unsigned int color )
{
	//pRenderer->DrawPolygon( points, nPoints, color );
}

void DrawPolygonOutlined( float* points, size_t nPoints, unsigned int color, float outlineWidth, unsigned int outlineColor )
{
	DrawPolygon( points, nPoints, color );
	DrawPolyLine( points, nPoints, outlineWidth, outlineColor, true );
}

void DrawCircle( float cx, float cy, float r, const mage::Color& color )
{
	const int segments = 20;
	const float inc = Mathf::TWO_PI / segments;
	float theta = 0;
	std::vector< float > polyLine;

	for ( int i = 0; i < segments; ++i )
	{
		theta += inc;
		float px = std::cosf( theta ) * r + cx;
		float py = std::sinf( theta ) * r + cy;
		polyLine.push_back( px );
		polyLine.push_back( py );
	}
	DrawPolyLine( polyLine.data(), polyLine.size() / 2, 1.0f, color.bgra, true );
}

void DrawText( float x, float y, Font* font, const mage::Color& color, const char* text )
{
	font->RenderTextDynamic( (int) x, (int) y, text, Color( color ), Font::Font_ULTRA );
}

void DrawText( float x, float y, const mage::Color& color, const char* text )
{
	DrawText( x, y, gDefaultFont, color, text );
}

void DrawText( float x, float y, const char* text )
{
	DrawText( x, y, gDefaultFont, Color::WHITE, text );
}

void DrawTextFormat( float x, float y, Font* font, const mage::Color& color, const char* text, ... )
{
	char textFormatBuffer[ 1024 ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, text );
	vsprintf_s( textFormatBuffer, text, vargs );
	va_end( vargs );

	DrawText( x, y, font, color, textFormatBuffer );
}

void DrawTextFormat( float x, float y, const mage::Color& color, const char* text, ... )
{
	char textFormatBuffer[ 1024 ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, text );
	vsprintf_s( textFormatBuffer, text, vargs );
	va_end( vargs );

	DrawText( x, y, gDefaultFont, color, textFormatBuffer );
}

void DrawTextFormat( float x, float y, const char* text, ... )
{
	char textFormatBuffer[ 1024 ];

	// Apply text formating
	va_list vargs;
	va_start( vargs, text );
	vsprintf_s( textFormatBuffer, text, vargs );
	va_end( vargs );

	DrawText( x, y, gDefaultFont, Color::WHITE, textFormatBuffer );
}

void SetViewMatrix( const float* view )
{
	IRenderCall( SetViewMatrix( view ) );
}

void ClearScreen()
{
	if ( gActiveSurface )
		gActiveSurface->Clear();
	else
		IRenderCall( ClearScreen() );
}

void CreateTexture( IRenderer::TextureHandle* hTexture, void* pixels, unsigned int w, unsigned int h, IRenderer::PixelFormat format, bool linearFilter )
{
	IRenderCall( CreateTexture( hTexture, pixels, w, h, format, linearFilter ) );
}

void FreeTexture( IRenderer::TextureHandle* hTexture )
{
	// Render should cleanup textures when it is destroyed
	// Textures may be freed at other times through this function
	if ( pRenderer )
		IRenderCall( FreeTexture( hTexture ) );
}

void SetClearColor( float r, float g, float b, float a )
{
	IRenderCall( SetClearColor( r, g, b, a ) );
}

void SetViewport( int x, int y, int w, int h )
{
	IRenderCall( SetViewport( x, y, w, h ) );
}

void SetOrthoView( float left, float right, float bottom, float top, float zNear, float zFar )
{
	Matrix4f view;
	view.MakeOrtho( left, right, top, bottom, zNear, zFar );
	SetViewMatrix( view.GetArray() );
}

void PushMatrix()
{
	gMatrixStack.push( gCurrentMatrix );
}

void PopMatrix()
{
	if ( !gMatrixStack.empty() )
	{
		gCurrentMatrix = gMatrixStack.top();
		gMatrixStack.pop();
	}
}

void LoadIdentity()
{
	gCurrentMatrix.Identity();
}

void LoadMatrix( float* m )
{
	gCurrentMatrix.Set( m );
}

void Translate( float x, float y, float z )
{
	Matrix4f m;//( gCurrentMatrix );
	m.Translate( Vec3f( x, y, z ) );
	gCurrentMatrix = m * gCurrentMatrix;
	//gCurrentMatrix *= m;
}

void Rotate( float angleDeg, float x, float y, float z )
{
	//Matrix4f m;( gCurrentMatrix );
	//m.Rotate( angleDeg, Vec3f( x, y, z ) );
	//gCurrentMatrix = m * gCurrentMatrix;

	Quatf q;
	q.FromAxisAngle( Vec3f( x, y, z ), Mathf::DegreesToRadians( angleDeg ) );
	Matrix4f R;
	R.RotationFromQuat( q );
	gCurrentMatrix = R * gCurrentMatrix;

	//gCurrentMatrix *= m;
}

void Scale( float x, float y, float z )
{
	Matrix4f m;//( gCurrentMatrix );
	m.Scale( Vec3f( x, y, z ) );
	gCurrentMatrix = m * gCurrentMatrix;
	//gCurrentMatrix *= m;
}

void ScaleUniform( float scale )
{
	Scale( scale, scale, scale );
}

void Translate2D( const Vec2f& trans )
{
	Translate2D( trans.x, trans.y );
}

void Translate2D( float x, float y )
{
	Translate( x, y, 0.0f );
}

void Rotate2D( float angleDeg )
{
	Rotate( angleDeg, 0.0f, 0.0f, 1.0f );
}

void Scale2D( float x, float y )
{
	Scale( x, y, 1.0f );
}

void ScaleUniform2D( float scale )
{
	Scale2D( scale, scale );
}

void SetActiveEffect( Effect* effect )
{
	IRenderCall( SetActiveEffect( effect ) );
}

void ClearActiveEffect()
{
	IRenderCall( ClearActiveEffect() );
}

void BindTexture( Texture* texture, int channel )
{
	IRenderer::TextureHandle hTexture = texture->GetId();
	IRenderCall( BindTexture( hTexture, channel ) );
}

void SetActiveSurface( Surface* surface )
{
	// Flush if we are switching draw surfaces
	if ( gActiveSurface != surface )
	{
		FlushRenderer();
	}
	gActiveSurface = surface;
	gActiveSurface->Bind();
}

void SetPickingID( int id )
{
	gPickingID = id;
}

}