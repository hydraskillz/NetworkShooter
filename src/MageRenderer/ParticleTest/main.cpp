#include <MageMath.h>
#include <MageCore.h>
#include <MageRenderer.h>
#include <SDL.h>

using namespace mage;

SpringGrid* grid;
bool gUseGrid;

const int NUM_GRAVITY_WELLS = 4;
Vec2f gGravityWells[NUM_GRAVITY_WELLS];

// Basic state for a particle
struct ParticleState
{
	Vec2f mVelocity;
	float mLengthMultiplier;

	static void UpdateParticle( ParticleManager< ParticleState >::Particle& particle, float dt )
	{
		ParticleState& ps = particle.mState;
		Vec2f& vel = ps.mVelocity;

		for ( int i = 0; i < NUM_GRAVITY_WELLS; ++i )
		{
			Vec2f diff = gGravityWells[i] - particle.mPosition;
			float d = diff.Length();
			Vec2f n = diff / d;
			vel += 10000 * n / ( d * d + 10000 );

			if ( d < 400 )
				vel += 45 * Vec2f( n.x, -n.y ) / ( d + 100 );
		}

		particle.mPosition += vel;
		particle.mRotation = vel.Angle();

		float speed = vel.Length();
		float alpha = Mathf::Min( 1.0f, Mathf::Min( particle.mLifePercent * 2.0f, speed ) );
		alpha *= alpha;

		particle.mColor.a = (uint8)( 255 * alpha );

		particle.mScale.x = ps.mLengthMultiplier * 
			Mathf::Min( Mathf::Min( 1.0f, 0.2f * speed + 0.1f ), alpha );

		// Stop particle if its moving really slow
		if ( fabs( vel.x ) + fabs( vel.y ) < 0.00000000001f )
			vel = Vec2f::ZERO;

		// Damping
		vel *= 0.97f;

		//grid->ApplyExplosionForce( 0.5f * vel.Length(), particle.mPosition, 80 );
		//if ( gUseGrid )
		//	grid->ApplyDirectionalForce( 0.5f * vel, particle.mPosition, 80 );
	}
};

void RandomizeGravityWells()
{
	for ( int i = 0; i < NUM_GRAVITY_WELLS; ++i )
	{
		if ( rand() & 1 )
			gGravityWells[i].x = RNG::RandomInRange( 100, 400 );
		else
			gGravityWells[i].x = RNG::RandomInRange( 600, 1000 );
		gGravityWells[i].y = RNG::RandomInRange( 100, 600 );
	}
}

Clock* mClock;
int mWindowWidth;
int mWindowHeight;

struct ExplosionManager
{
	ExplosionManager()
		: particleManager( 1024 * 20, ParticleState::UpdateParticle )
		, particleTex( 0 )
	{}
	~ExplosionManager()
	{
		Delete0( particleTex );
	}

	ParticleManager< ParticleState > particleManager;
	Texture* particleTex;

	void Init()
	{
		particleTex = new Texture( "../data/particleLine.png" );
		particleTex->Load();
	}

	void Update( float dt )
	{
		particleManager.OnUpdate( dt );
	}

	void Draw()
	{
		particleManager.OnDraw();
	}

	void DoExplosion( Dictionary& )
	{
		// Do an explosion!
		float hue1 = RNG::RandomInRange< float >( 0, 6 );
		float hue2 = std::fmod( ( hue1 + RNG::RandomInRange< float >( 0, 2 ) ), 6 );
		Color color1 = Color::HSVToColor( hue1, 0.5f, 1 );
		Color color2 = Color::HSVToColor( hue2, 0.5f, 1 );
		Vec2f pos =  Vec2f( 
			RNG::RandomInRange( 0, mWindowWidth ),
			RNG::RandomInRange( 0, mWindowHeight ) );

		for ( int i = 0; i < 120; ++i )
		{
			float speed = 20.0f * ( 1.0f - 1.0f / RNG::RandomInRange( 1.0f, 10.0f ) );
			//ConsolePrintf( "Speed: %f\n", speed );
			ParticleState state;
			state.mVelocity = Vec2f(
				RNG::RandomInRange( -speed, speed ),
				RNG::RandomInRange( -speed, speed ) );
			state.mLengthMultiplier = 1.0f;
			Color color = Color::Lerp( color1, color2, RNG::RandomUnit() );

			particleManager.CreateParticle( particleTex, pos,
				color, 2.5f, state, Vec2f::ONE, 3.14f );
		}

		grid->ApplyExplosionForce( RNG::RandomInRange( 50, 100 ), pos, 80 );

		mClock->PostEventCallbackAfter( "DoExplosion", 0.5 );
	}
};

int main( int argc, char** argv )
{
	SDL_Surface* mDisplaySurface = NULL;
	mWindowWidth = 1280;
	mWindowHeight = 720;
	Surface* scene;
	Surface* pass0;
	bool mRunning = true;
	SDL_Event sdlEvent;
	float dt = 1.0f / 60.0f;
	bool useGlow = true;
	bool useWells = true;
	bool doExplosion = true;

	mClock = &Clock::Initialize();
	
	float points = 560;
	Vec2f gridSpacing(
		std::sqrtf( mWindowWidth * mWindowHeight / points ),
		std::sqrtf( mWindowWidth * mWindowHeight / points ));
	grid = new SpringGrid( RectI( 0, 0, mWindowWidth, mWindowHeight ), gridSpacing );
	grid->ScreenSize.Set( mWindowWidth, mWindowHeight );

	// Cool glow effect!
	int bloomPrest = 1;
	BloomEffect bloomEffect;
	bloomEffect.Settings = BloomSettings::Presets[ BloomSettings::BLOOM_SOFT ];

	// Initialize SDL
	if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
		return 1;
	// Center the window
	SDL_putenv( "SDL_VIDEO_CENTERED=center" );
	// Set the video flags
	Uint32 flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF;
	// Create the window
	mDisplaySurface = SDL_SetVideoMode( mWindowWidth, mWindowHeight, 32, flags );
	if ( !mDisplaySurface )
		return 1;
	if ( !CreateRenderer() )
	{
		printf( "Failed to create renderer!\n" );
	}

	scene = new Surface( mWindowWidth, mWindowHeight, true, true, true );
	scene->ClearColor = Color::BLACK;
	pass0 = new Surface( mWindowWidth, mWindowHeight, false, true );

	mClock->PostEventCallbackAfter( "DoExplosion", 0.5 );
	ExplosionManager explosionManager;
	explosionManager.Init();
	EventManager::RegisterObjectForEvent( "DoExplosion", explosionManager, &ExplosionManager::DoExplosion );

	bloomEffect.Initialize( mWindowWidth / 2, mWindowHeight / 2 );

	RNG::RandomInRange(1, 10 );

	RandomizeGravityWells();

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
					if ( sdlEvent.key.keysym.sym == SDLK_r )
					{
						RandomizeGravityWells();
						break;
					}
					if ( sdlEvent.key.keysym.sym == SDLK_g )
					{
						useGlow = !useGlow;
						break;
					}
					if ( sdlEvent.key.keysym.sym == SDLK_h )
					{
						gUseGrid = !gUseGrid;
						break;
					}
					if ( sdlEvent.key.keysym.sym == SDLK_w )
					{
						useWells = !useWells;
						break;
					}
					if ( sdlEvent.key.keysym.sym == SDLK_b )
					{
						bloomEffect.Settings = BloomSettings::Presets[ (++bloomPrest) % BloomSettings::BLOOM_COUNT ];
						break;
					}
					if ( sdlEvent.key.keysym.sym == SDLK_SPACE )
					{
						doExplosion = !doExplosion;
						
						break;
					}
					break;
				}

				case SDL_QUIT:
					mRunning = false;
					break;
			}
		}

		if ( doExplosion )
		{
			mClock->AdvanceTime( dt );
		}

		explosionManager.Update( dt );
		if ( gUseGrid )
			grid->OnUpdate( dt );
		
		SetActiveSurface( pass0 );
		ClearScreen();

		if ( gUseGrid )
			grid->OnDraw();

		for ( int i = 0; i < NUM_GRAVITY_WELLS; ++i )
		{
			DrawRect( gGravityWells[i].x, gGravityWells[i].y, 10, 10, Color::WHITE.bgra );
			if ( gUseGrid && useWells )
				grid->ApplyImplosionForce( 15, gGravityWells[i], 200 );
		}

		explosionManager.Draw();

		FlushRenderer();

		if ( useGlow )
			bloomEffect.Draw( pass0, scene );
		else
		{
			SetActiveSurface( scene );
			ClearScreen();
			DrawRect( pass0->TextureColor, 0, 0, scene->Width, scene->Height, Color::WHITE, true );
			FlushRenderer();
		}

		SDL_GL_SwapBuffers();
	}

	return 0;
}