#include "main.h"

using namespace mage;

IPaddress gServerAddr;
LocalClient gClient;
int gClientIndex;
Player* gLocalPlayer;
bool gClientPosChanged;
float gLastFireTime;

// Effects
SpringGrid* gGrid;
BloomEffect gGlow;
Surface* gScene;
Surface* gPass0;

#include "ExplosionEffect.cpp"

ExplosionManager gExplosionMangaer;

#define LocalPlayerAliveAndWell ( gLocalPlayer && gLocalPlayer->alive )

//--------------------------------------
void InitClient( const std::string& addr )
{
	InitApp( "Simple Net Game - Client", 800, 600 );

	// Init renderer
	InitGraphics();

	// Setup view
	SetViewport( 0, 0, 800, 600 );
	SetOrthoView( 0, 800, 600, 0, 0, 1 );

	// Create clock
	gClock = &Clock::Initialize();

	// Register callbacks
	RegisterUpdateFn( ClientUpdate );
	RegisterInputFn( ClientInput );

	// Events
	EventManager::RegisterFunctionForEvent( "ClientSendPos", ClientSendPos );
	EventManager::RegisterFunctionForEvent( "PlayerThrust", PlayerThrust );
	EventManager::RegisterFunctionForEvent( "PlayerSteer", PlayerSteer );
	EventManager::RegisterFunctionForEvent( "Fire", ClientFire );
	EventManager::RegisterFunctionForEvent( "SpawnBullet", ClientSpawnBullet );

	// Setup net session
	gSession = new NetSession();
	gSession->VerboseDebugMsg = VERBOSE_NET_DEBUG;
	gSession->RegisterClientConnectCallback( OnServerConnect );
	gSession->RegisterClientDisconnectCallback( OnServerLost );
	gSession->SetMaxPacketSize( 2048 );

	// Effects
	float points = 560;
	Vec2f gridSpacing(
		std::sqrtf( 600 * 600 / points ),
		std::sqrtf( 800 * 800 / points ));
	gGrid = new SpringGrid( RectI( 0, 0, 800, 600 ), gridSpacing );
	gGrid->ScreenSize.Set( 800, 600 );

	gGlow.Settings = BloomSettings::Presets[ BloomSettings::BLOOM_SOFT ];
	gGlow.Initialize( 400, 300 );

	gScene = new Surface( 800, 600, true, true, true );
	gScene->ClearColor = Color::BLACK;
	gPass0 = new Surface( 800, 600, false, true );

	gExplosionMangaer.Init();

	gClient = gSession->CreateLocalClient();
	gServerAddr = gClient.ConnectTo( addr.c_str(), 5000 );

	gClientIndex = -1;
	gLocalPlayer = 0;
	gClientPosChanged = false;
	gLastFireTime = 0;
	RNG::SetRandomSeed( (unsigned long) time(0) );
}
//--------------------------------------
void OnClientExit()
{
	gClient.Disconnect();
	delete gSession;
}
//--------------------------------------
void ClientUpdate( float dt )
{
	// Advance time
	gClock->AdvanceTime( dt );

	// Update players
	UpdatePlayers( gPlayers, dt );

	if ( gClientIndex >= 0 && gPlayers[ gClientIndex ].vel.LengthSqr() != 0 )
		gClientPosChanged = true;

	// Update network
	gSession->OnUpdate();

	gLastFireTime = Mathf::Max( gLastFireTime - dt, 0 );

	NetClient server;
	uint32 commands_in;
	Player* player;

	while ( gClient.IsDataReady() )
	{
		// Get next packet
		gClient.ReceiveData( gReader, server );

		commands_in = gReader.Read< uint32 >();

		// Our index
		if ( commands_in & NC_PLAYER_ID )
		{
			gClientIndex = gReader.Read< int >();
			gLocalPlayer = &gPlayers[ gClientIndex ];

			ConsolePrintf( "Client : I am player %d\n", gClientIndex );

			gClock->PostEventCallbackAfter( "ClientSendPos", 0.05 );
		}

		// Player(s) joined
		if ( commands_in & NC_ADD )
		{
			int num = gReader.Read< int >();
			ConsolePrintf( "Client : ADD command %d\n", num );
			for ( int i = 0; i < num; ++i )
			{
				int index = DeserializePlayer( gPlayers, gReader );

				player = &gPlayers[ index ];
				player->active = 1;

				ConsolePrintf( "Client : Player joined id=%d name=%s\n", index, player->name );

				gGrid->ApplyDirectionalForce( Vec3f( 0, 0, 5000 ), Vec3f( player->pos.x, player->pos.y, 0 ), 80 );
			}
		}

		// Player(s) left
		if ( commands_in & NC_REMOVE )
		{
			int num = gReader.Read< int >();
			ConsolePrintf( "Client : REMOVE command %d\n", num );
			for ( int i = 0; i < num; ++i )
			{
				int index = gReader.Read< int >();
				player = &gPlayers[ index ];
				player->active = 0;
				player->ID = 0;

				ConsolePrintf( "Client : Player left id=%d name=%s\n", index, player->name );
			}
		}

		// Location info
		if ( commands_in & NC_LOCATION )
		{
			int num = gReader.Read< int >();
			for ( int i = 0; i < num; ++i )
			{
				int index = gReader.Read< int >();
				player = &gPlayers[ index ];
				player->pos = gReader.Read< Vec2f >();
				player->rotation = gReader.Read< float >();

				gGrid->ApplyExplosionForce( 40, player->pos, 20 );
			}
		}

		// Fire info
		if ( commands_in & NC_FIRE )
		{
			int who = gReader.Read< int >();
			int bulletIndex = gReader.Read< int >();
			Vec2f bulletVel = gReader.Read< Vec2f >();

			Dictionary params;

			params.Set( "PlayerIndex", who );
			params.Set( "BulletIndex", bulletIndex );
			params.Set( "BulletVel", bulletVel );

			// Fire bullet in 100ms - rtt to server
			gClock->PostEventCallbackAfter( "SpawnBullet",
				Mathd::Max( 0.0, 0.1 - gSession->GetAverageRTT( gClient.GetID() ) ),
				params );
		}

		// Kill info
		if ( commands_in & NC_KILL )
		{
			int killer = gReader.Read< int >();
			int killed = gReader.Read< int >();
			gPlayers[killed].alive = 0;
			gPlayers[killed].timeToRespawn = RESPAWN_TIME;

			gExplosionMangaer.DoExplosion( gPlayers[killed].pos, GetColorForPlayer( killed ) );

			ConsolePrintf( "%s(%d) killed %s(%d)\n", gPlayers[killer].name, killer, gPlayers[killed].name, killed );
		}

		// Respawn info
		if ( commands_in & NC_RESPAWN )
		{
			int who = gReader.Read< int >();
			gPlayers[who].alive = 1;
			gPlayers[who].pos = gReader.Read< Vec2f >();
			gPlayers[who].rotation = 0;
			gPlayers[who].vel = Vec2f::ZERO;

			gGrid->ApplyDirectionalForce( Vec3f( 0, 0, 5000 ), Vec3f( gPlayers[who].pos.x, gPlayers[who].pos.y, 0 ), 80 );
		}
	}

	gGrid->OnUpdate( dt );
	gExplosionMangaer.OnUpdate( dt );

	// Render
	SetActiveSurface( gPass0 );
	ClearScreen();

	gGrid->OnDraw();
	gExplosionMangaer.OnDraw();
	
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		player = &gPlayers[i];
		if ( player->active && player->alive )
		{
			Color playerColor = GetColorForPlayer( i );

			PushMatrix();

			Translate2D( player->pos );
			::Rotate( Mathf::RadiansToDegrees( player->rotation ), 0, 0, -1 );
			Scale( 1.5f, 1.5f, 1.0f );

			const float a = 0;
			const float b = 5;
			const float c = 4;
			const float d = 2;

			DrawLine(  a,  b,  c, -b, 1.0f, playerColor );
			DrawLine(  c, -b,  a, -d, 1.0f, playerColor );
			DrawLine(  a, -d, -c, -b, 1.0f, playerColor );
			DrawLine( -c, -b,  a,  b, 1.0f, playerColor );

			PopMatrix();

			for ( int j = 0; j < MAX_BULLETS; ++j )
			{
				Bullet* b = &player->bullets[j];
				if ( b->active )
				{
					DrawCircle( b->pos.x, b->pos.y, BULLET_RADIUS, GetColorForPlayer( i ) );

					gGrid->ApplyExplosionForce( 40, b->pos, 20 );
				}
			}
		}
	}

	FlushRenderer();

	gGlow.Draw( gPass0, gScene );

	SetActiveSurface( gScene );

	/*
	DrawTextFormat( 0, 0, "Local Time %s",
		Clock::GetStringFormatedTime( gClock->GetElapsedTime( Clock::TIME_SEC ) ).c_str() );
	DrawTextFormat( 0, 14, "Server Time %s",
		Clock::GetStringFormatedTime( gSession->GetNetTimeSeconds() ).c_str() );*/
	
	if ( LocalPlayerAliveAndWell )
	{
		DrawTextFormat( 0, 0, "Ping: %d", gSession->GetAverageRTT( gLocalPlayer->ID ) * 1000 );
	}

	DrawPlayerNames( gPlayers, 10, 28 );

	FlushRenderer();
}
//--------------------------------------
bool ClientInput( SDL_Event& sdlEvent )
{
	return DefaultInputFn( sdlEvent );
}
//--------------------------------------
void OnServerConnect( clientID_t clientID, IPaddress clientAddr )
{
	ConsolePrintf( "Client : Connected to %u\n", clientID );
	char name[ MAX_NAME_LEN ];
	if ( NetManager::GetHostName( name, MAX_NAME_LEN ) != 0 )
		SendHello( "Player" );
	else
		SendHello( name );
}
//--------------------------------------
void OnServerLost( clientID_t clientID, IPaddress clientAddr )
{
	ConsolePrintf( "Client : Disconnected from %u\n", clientID );
}
//--------------------------------------
void SendHello( const char* name )
{
	uint32 commands = 0;

	// Init all players slots to none
	memset( gPlayers, NULL, sizeof( Player ) * MAX_PLAYERS );

	ConsolePrintf( "Client : Using name '%s'\n", name );

	commands |= NC_HELLO;
	commands |= NC_NAME;

	gWriter.Clear();

	gWriter.Write( commands );
	gWriter.WriteString( name );

	gClient.SendData( gWriter, gServerAddr );
}
//--------------------------------------
void ClientSendPos( Dictionary& params )
{
	// Call this function again after a delay
	gClock->PostEventCallbackAfter( "ClientSendPos", 0.05 );

	// Send position packet if needed
	if ( gClientPosChanged )
	{
		uint32 command = 0;

		gClientPosChanged = false;

		command |= NC_LOCATION;

		gWriter.Write( command );
		gWriter.Write( gLocalPlayer->pos );
		gWriter.Write( gLocalPlayer->rotation );

		gClient.SendData( gWriter, gServerAddr );
	}
}
//--------------------------------------
void PlayerSteer( Dictionary& params )
{
	float x = 0;

	if ( LocalPlayerAliveAndWell )
	{
		params.Get( "x", x );

		gLocalPlayer->rotation += x;
		gClientPosChanged = true;
	}
}
//--------------------------------------
void PlayerThrust( Dictionary& params )
{
	float x = 0;
	const float radians = gLocalPlayer->rotation;

	const float cosRot = std::cos( radians );
	const float sinRot = std::sin( radians );

	if ( LocalPlayerAliveAndWell )
	{
		params.Get( "x", x );

		gLocalPlayer->vel += Vec2f( sinRot * x, cosRot * x );
		gClientPosChanged = true;
	}
}
//--------------------------------------
void ClientFire( Dictionary& params )
{
	if ( LocalPlayerAliveAndWell )
	{
		if ( gLastFireTime == 0 )
		{
			gLastFireTime = 0.2f;

			uint32 commands = NC_FIRE;
			gWriter.Write( commands );
			gWriter.Write( gLocalPlayer->index );

			gClient.SendData( gWriter, gServerAddr );
		}
	}
}
//--------------------------------------
void ClientSpawnBullet( Dictionary& params )
{
	int who;
	int bulletIndex;
	Vec2f bulletVel;

	params.Get( "PlayerIndex", who );
	params.Get( "BulletIndex", bulletIndex );
	params.Get( "BulletVel", bulletVel );

	Bullet& b = gPlayers[ who ].bullets[ bulletIndex ];
	b.active = 1;
	b.index = bulletIndex;
	b.ownerIndex = who;
	b.pos = gPlayers[ who ].pos;
	b.vel = bulletVel;
	b.lifetime = BULLET_LIFE;
}
//--------------------------------------