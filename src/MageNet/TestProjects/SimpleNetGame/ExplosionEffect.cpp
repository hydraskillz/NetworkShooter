#include "main.h"

struct ParticleState
{
	Vec2f mVelocity;
	float mLengthMultiplier;

	static void UpdateParticle( ParticleManager< ParticleState >::Particle& particle, float dt )
	{
		ParticleState& ps = particle.mState;
		Vec2f& vel = ps.mVelocity;

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
	}
};

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

	void OnUpdate( float dt )
	{
		particleManager.OnUpdate( dt );
	}

	void OnDraw()
	{
		particleManager.OnDraw();
	}

	void DoExplosion( const Vec2f& pos, const Color& color )
	{
		// Do an explosion!
		//float hue1 = RNG::RandomInRange< float >( 0, 6 );
		//float hue2 = std::fmod( ( hue1 + RNG::RandomInRange< float >( 0, 2 ) ), 6 );
		//Color color1 = Color::HSVToColor( hue1, 0.5f, 1 );
		//Color color2 = Color::HSVToColor( hue2, 0.5f, 1 );

		for ( int i = 0; i < 120; ++i )
		{
			float speed = 20.0f * ( 1.0f - 1.0f / RNG::RandomInRange( 1.0f, 10.0f ) );
			//ConsolePrintf( "Speed: %f\n", speed );
			ParticleState state;
			state.mVelocity = Vec2f(
				RNG::RandomInRange( -speed, speed ),
				RNG::RandomInRange( -speed, speed ) );
			state.mLengthMultiplier = 1.0f;
			//Color color = Color::Lerp( color1, color2, RNG::RandomUnit() );

			particleManager.CreateParticle( particleTex, pos,
				color, 2.5f, state, Vec2f::ONE, 3.14f );
		}

		gGrid->ApplyExplosionForce( RNG::RandomInRange( 50, 100 ), pos, 80 );
	}
};