#include "main.h"

//--------------------------------------
void InitGraphics()
{
	// Init renderer
	CreateRenderer();
}
//--------------------------------------
Player* GetPlayerByID( Player* players, clientID_t id )
{
	Player* p = NULL;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( players[ i ].ID == id )
		{
			p = &players[ i ];
			break;
		}
	}
	return p;
}
//--------------------------------------
int GetNumActivePlayers( Player* players )
{
	int n = 0;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( players[ i ].active )
			++n;
	}
	return n;
}
//--------------------------------------
Color GetColorForPlayer( int index )
{
	static Color playerColors[] =
	{
		Color::WHITE,
		Color::GREEN,
		Color::CYAN,
		Color::RED,
		Color::PINK,
		Color::GREY,
		Color::DARK_GREEN,
		Color::BLUE,
		Color::DARK_RED,
		Color::PURPLE,
	};
	return playerColors[ index ];
}
//--------------------------------------
void SerializePlayer( Player* player, PacketWriter& writer )
{
	writer.Write( player->index );
	writer.WriteString( player->name );
	writer.Write( player->pos );
	writer.Write( player->vel );
	writer.Write( player->rotation );
	writer.Write( player->alive );
	for ( int i = 0; i < MAX_BULLETS; ++i )
	{
		writer.Write( player->bullets[i].active );
		writer.Write( player->bullets[i].pos );
		writer.Write( player->bullets[i].vel );
		writer.Write( player->bullets[i].lifetime );
	}
}
//--------------------------------------
int DeserializePlayer( Player* players, PacketReader& reader )
{
	Player* player;
	int index;

	index = reader.Read< int >();
	player = &players[ index ];

	player->index = index;
	reader.ReadString( player->name, MAX_NAME_LEN );
	player->pos = reader.Read< Vec2f >();
	player->vel = reader.Read< Vec2f >();
	player->rotation = reader.Read< float >();
	player->alive = reader.Read< int >();
	player->killedBy = -1;
	for ( int i = 0; i < MAX_BULLETS; ++i )
	{
		player->bullets[i].active      = reader.Read< int >();
		player->bullets[i].pos         = reader.Read< Vec2f >();
		player->bullets[i].vel         = reader.Read< Vec2f >();
		player->bullets[i].lifetime	   = reader.Read< float >();
		player->bullets[i].ownerIndex  = player->index;
		player->bullets[i].index       = i;
	}

	return index;
}
//--------------------------------------
void UpdatePlayers( Player* players, float dt )
{
	Player* player;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		player = &players[i];

		// Update player
		if ( player->active && player->alive )
		{
			player->pos += player->vel * dt;
			player->vel *= 0.98f;

			if ( player->pos.x > 800 ) player->pos.x = 0;
			if ( player->pos.x < 0 )   player->pos.x = 800;
			if ( player->pos.y > 600 ) player->pos.y = 0;
			if ( player->pos.y < 0 )   player->pos.y = 600;

			// Update players bullets
			for ( int j = 0; j < MAX_BULLETS; ++j )
			{
				Bullet* b = &player->bullets[j];
				if ( b->active )
				{
					b->pos += b->vel * dt;

					if ( b->pos.x > 800 ) b->pos.x = 0;
					if ( b->pos.x < 0 )   b->pos.x = 800;
					if ( b->pos.y > 600 ) b->pos.y = 0;
					if ( b->pos.y < 0 )   b->pos.y = 600;

					for ( int k = 0; k < MAX_PLAYERS; ++k )
					{
						if ( k != i )
						{
							Player* otherP = &players[k];
							Vec2f d = otherP->pos - b->pos;
							float radius = BULLET_RADIUS * PLAYER_RADIUS;
							if ( d.LengthSqr() < radius * radius && otherP->alive )
							{
								b->active = false;
								if ( otherP->killedBy < 0 )
									otherP->killedBy = b->ownerIndex;
							}
						}
					}

					// Kill bullets after some time
					b->lifetime = Mathf::Max( 0, b->lifetime - dt );
					if ( b->lifetime == 0 )
					{
						b->active = false;
					}
				}
			}
		}
		else if ( player->active )
		{
			player->timeToRespawn = Mathf::Max( 0, player->timeToRespawn - dt );
		}
	}
}
//--------------------------------------
void DrawPlayerNames( Player* payers, float x, float y )
{
	DrawRect( x, y, 128, 14 * MAX_PLAYERS, Color( 0x88000000 ) );

	Player* player;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		player = &payers[i];
		if ( player->active )
		{
			DrawTextFormat( x, y, GetColorForPlayer( i ), "%s%s%s"
				, payers[i].name
				, payers[i].alive ? "" : " Respawn in "
				, payers[i].alive ? "" : Clock::GetStringFormatedTime( player->timeToRespawn ).c_str() );
			y += 14;
		}
	}
}
//--------------------------------------