#include "main.h"

using namespace mage;

LocalClient gServer;

//--------------------------------------
void InitServer()
{
	InitApp( "Simple Net Game - Server", 300, 400 );

	InitGraphics();
	SetViewport( 0, 0, 300, 400 );
	SetOrthoView( 0, 300, 400, 0, 0, 1 );

	gClock = &Clock::Initialize();

	RegisterUpdateFn( ServerUpdate );

	EventManager::RegisterFunctionForEvent( "RespawnPlayer", RespawnPlayer );

	gSession = new NetSession();
	gSession->OpenPort( 5000 );
	gSession->VerboseDebugMsg = VERBOSE_NET_DEBUG;
	gSession->SetMaxPacketSize( 2048 );

	gServer = gSession->CreateLocalClient();
	
	gSession->RegisterClientConnectCallback( OnNewClient );
	gSession->RegisterClientDisconnectCallback( OnLostClient );
}
//--------------------------------------
void OnServerExit()
{
	gSession->DropAllClients();
	delete gSession;
}
//--------------------------------------
void ServerUpdate( float dt )
{
	NetClient client;
	uint32 commands_in;
	uint32 commands_out;
	Player* player;
	Bullet* b;

	// Advance time
	gClock->AdvanceTime( dt );

	// Update players
	UpdatePlayers( gPlayers, dt );

	// Update network
	gSession->OnUpdate();

	// Check packets
	while ( gServer.IsDataReady() )
	{
		// Get next packet
		gServer.ReceiveData( gReader, client );

		// Get player for this client
		player = GetPlayerByID( gPlayers, client.GetID() );

		// This client didn't connect correctly!
		if ( !player )
		{
			ConsolePrintf( CONSOLE_WARNING, "Server : Unknown client %u\n", client.GetID() );
			continue;
		}

		// Read commands
		commands_in = gReader.Read< uint32 >();
		commands_out = 0;

		// Client id message
		if ( commands_in & NC_HELLO )
		{
			commands_out |= NC_PLAYER_ID;
			commands_out |= NC_ADD;
		}

		// Client name message
		if ( commands_in & NC_NAME )
		{
			gReader.ReadString( player->name, MAX_NAME_LEN );
			ConsolePrintf( "Got name msg: %s\n", player->name );
		}

		// Client position update
		if ( commands_in & NC_LOCATION )
		{
			player->pos = gReader.Read< Vec2f >();
			player->rotation = gReader.Read< float >();
			
			// Replicate location message to other clients
			gWriter.Write( (uint32) NC_LOCATION );
			gWriter.Write( 1 );
			gWriter.Write( player->index );
			gWriter.Write( player->pos );
			gWriter.Write( player->rotation );
			for ( int i = 0; i < MAX_PLAYERS; ++i )
			{
				if ( gPlayers[i].active && i != player->index )
				{
					gServer.SendData( PacketWriter( gWriter ), gPlayers[i].address );
				}
			}
			gWriter.Clear();
		}

		// Client requesting to fire
		if ( commands_in & NC_FIRE )
		{
//			ConsolePrintf( "Server : Got fire command from %u\n", client.GetID() );
			b = ServerFire( player );
			if ( b )
			{
				// Replicate fire message to other clients
				gWriter.Write( (uint32) NC_FIRE );
				gWriter.Write( player->index );
				gWriter.Write( b->index );
				gWriter.Write( b->vel );
				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					if ( gPlayers[i].active )
					{
						gServer.SendData( PacketWriter( gWriter ), gPlayers[i].address );
					}
				}
				gWriter.Clear();
			}
		}

		// Send commands back to client
		if ( commands_out )
		{
			gWriter.Write( commands_out );

			// ID the player
			if ( commands_out & NC_PLAYER_ID )
			{
				gWriter.Write( player->index );
			}

			// Tell the client about all clients (including them)
			if ( commands_out & NC_ADD )
			{
				gWriter.Write( GetNumActivePlayers( gPlayers ) );
				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					if ( gPlayers[i].active )
					{
						SerializePlayer( &gPlayers[i], gWriter );
					}
				}
			}

			// Send command to client
			gServer.SendData( gWriter, client.Address, SENDOPT_RELIABLE );

			// Tell all other clients about the new player
			if ( commands_out & NC_ADD )
			{
				commands_out = NC_ADD;
				gWriter.Write( commands_out );
				gWriter.Write( 1 );

				SerializePlayer( player, gWriter );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					if ( gPlayers[i].active && i != player->index )
					{
						gServer.SendData( PacketWriter( gWriter ), gPlayers[i].address, SENDOPT_RELIABLE );
					}
				}
				gWriter.Clear();
			}
		}
	}

	// Process kills
	commands_out = 0;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		player = &gPlayers[i];
		if ( player->active && player->killedBy >= 0 )
		{
			if ( commands_out == 0 )
			{
				commands_out = NC_KILL;
				gWriter.Write( commands_out );
			}

			gWriter.Write( player->killedBy );
			gWriter.Write( player->index );

			player->killedBy = -1;
			player->alive = 0;

			Dictionary params;
			params.Set( "PlayerIndex", i );
			gClock->PostEventCallbackAfter( "RespawnPlayer", RESPAWN_TIME, params );
		}
	}
	if ( commands_out )
	{
		gServer.SendData( gWriter );
	}

	// Render
	ClearScreen();

	DrawTextFormat( 0, 0, "Time %s",
		Clock::GetStringFormatedTime( gClock->GetElapsedTime( Clock::TIME_SEC ) ).c_str() );

	DrawPlayerNames( gPlayers, 10, 28 );

	FlushRenderer();
}
//--------------------------------------
void OnNewClient( clientID_t clientID, IPaddress clientAddr )
{
	ConsolePrintf( "Server : New client %u\n", clientID );

	int index;

	// Find empty player slot
	for ( index = 0; index < MAX_PLAYERS; ++index )
	{
		if ( gPlayers[ index ].active == 0 )
			break;
	}

	// No room - kick them
	if ( index == MAX_PLAYERS )
	{
		ConsolePrintf( CONSOLE_WARNING, "Server : No empty slots - dropping new client\n" );
		gSession->DropClient( clientID );
	}
	else
	{
		ConsolePrintf( "Server : Added client as %d\n", index );

		Player& p = gPlayers[ index ];

		p.active = 1;
		p.address = clientAddr;
		p.ID = clientID;
		p.index = index;
		p.pos = RNG::RandomInRange( Vec2f( 100, 100 ), Vec2f( 700, 500 ) );
		p.rotation = 0;
		p.alive = 1;
		p.killedBy = -1;
		memset( p.bullets, 0, sizeof( Bullet ) * MAX_BULLETS );
	}
}
//--------------------------------------
void OnLostClient( clientID_t clientID, IPaddress clientAddr )
{
	ConsolePrintf( "Server : client dropped %u\n", clientID );

	Player* player = GetPlayerByID( gPlayers, clientID );

	if ( player )
	{
		player->active = 0;

		// Tell all other clients about the dropped player
		int commands_out = NC_REMOVE;
		gWriter.Write( commands_out );

		gWriter.Write( 1 );
		gWriter.Write( player->index );
		for ( int i = 0; i < MAX_PLAYERS; ++i )
		{
			if ( gPlayers[i].active && i != player->index )
			{
				gServer.SendData( PacketWriter( gWriter ), gPlayers[i].address, SENDOPT_RELIABLE );
			}
		}
		gWriter.Clear();
	}
}
//--------------------------------------
Bullet* ServerFire( Player* player )
{
	const float cosRot = std::cos( player->rotation );
	const float sinRot = std::sin( player->rotation );
	Vec2f vel( sinRot * BULLET_SPEED + player->vel.x, cosRot * BULLET_SPEED + player->vel.y );
	Bullet* b = 0;
	for ( int i = 0; i < MAX_BULLETS && player->alive; ++i )
	{
		if ( !player->bullets[i].active )
		{
			b = &player->bullets[i];
			b->active = 1;
			b->index = i;
			b->pos = player->pos;
			b->vel = vel;
			b->lifetime = BULLET_LIFE;
			b->ownerIndex = player->index;
			break;
		}
	}
	return b;
}
//--------------------------------------
void RespawnPlayer( Dictionary& params )
{
	int who = -1;
	Player* player;

	params.Get( "PlayerIndex", who );

	if ( who >= 0 )
	{
		player = &gPlayers[ who ];

		// Make sure player is still connected
		if ( player->active )
		{
			player->alive = 1;
			player->pos = RNG::RandomInRange( Vec2f( 100, 100 ), Vec2f( 700, 500 ) );
			player->rotation = 0;
			player->vel = Vec2f::ZERO;
			
			gWriter.Write( NC_RESPAWN );
			gWriter.Write( who );
			gWriter.Write( player->pos );
			gServer.SendData( gWriter );
		}
	}
}
//--------------------------------------