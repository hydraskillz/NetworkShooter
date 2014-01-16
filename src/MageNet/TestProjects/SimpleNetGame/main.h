#pragma once

#include <MageMath.h>
#include <MageCore.h>
#include <MageRenderer.h>
#include <MageNet.h>
#include <MageApp.h>

using namespace mage;

//--------------------------------------
// Macros
//--------------------------------------
#define VERBOSE_NET_DEBUG false
#define MAX_NAME_LEN 24
#define MAX_PLAYERS 10
#define MAX_BULLETS 5
#define BULLET_SPEED 500
#define BULLET_LIFE 2.0f
#define BULLET_RADIUS 5
#define PLAYER_RADIUS 3
#define RESPAWN_TIME 2.0f
//--------------------------------------


//--------------------------------------
// Types
//--------------------------------------
struct Bullet
{
	int ownerIndex;
	int active;
	int index;
	Vec2f pos;
	Vec2f vel;
	float lifetime;
};
//--------------------------------------
struct Player
{
	clientID_t ID;
	IPaddress address;
	int active;
	int index;
	char name[ MAX_NAME_LEN ];
	Vec2f pos;
	Vec2f vel;
	float rotation;
	int killedBy;
	int alive;
	float timeToRespawn;
	Bullet bullets[ MAX_BULLETS ];
};
//--------------------------------------


//--------------------------------------
// Network commands
//--------------------------------------
enum NetCommand
{
	NC_HELLO		= 0x0001,				// No data
	NC_PLAYER_ID	= 0x0002,				// int
	NC_NAME		    = 0x0004,				// char* (null terminated)
	NC_ADD			= 0x0008,				// int (count) int (playerIndex) char* (name)
	NC_REMOVE		= 0x0010,				// int (count) int (playerIndex)
	NC_LOCATION		= 0x0020,				// int (count) int (playerIndex) vec2f (pos) float (rotation)
	NC_FIRE			= 0x0040,				// int (playerIndex) int (bulletIndex)
	NC_KILL			= 0x0080,				// int (killerIndex) int (killedIndex)
	NC_RESPAWN		= 0x0100,				// int (playerIndex)
};
//--------------------------------------


//--------------------------------------
// Globals
//--------------------------------------
static NetSession* gSession;
static PacketWriter gWriter;
static PacketReader gReader;
static Clock* gClock;
static Player gPlayers[ MAX_PLAYERS ];
//--------------------------------------


//--------------------------------------
// General
//--------------------------------------
void InitGraphics();
Player* GetPlayerByID( Player* players, clientID_t id );
int GetNumActivePlayers( Player* players );
Color GetColorForPlayer( int index );
void SerializePlayer( Player* player, PacketWriter& writer );
int DeserializePlayer( Player* players, PacketReader& reader );
void UpdatePlayers( Player* players, float dt );
void DrawPlayerNames( Player* payers, float x, float y );
//--------------------------------------


//--------------------------------------
// Server
//--------------------------------------
void InitServer();
void OnServerExit();
void ServerUpdate( float dt );
void OnNewClient( clientID_t clientID, IPaddress clientAddr );
void OnLostClient( clientID_t clientID, IPaddress clientAddr );
Bullet* ServerFire( Player* player );
void RespawnPlayer( Dictionary& params );
//--------------------------------------


//--------------------------------------
// Client
//--------------------------------------
void InitClient( const std::string& addr );
void OnClientExit();
void ClientUpdate( float dt );
bool ClientInput( SDL_Event& sdlEvent );
void OnServerConnect( clientID_t clientID, IPaddress clientAddr );
void OnServerLost( clientID_t clientID, IPaddress clientAddr );
void SendHello( const char* name );
void ClientSendPos( Dictionary& params );
void PlayerSteer( Dictionary& params );
void PlayerThrust( Dictionary& params );
void ApplyPlayerMotion( float dx, float dy );
void ClientFire( Dictionary& params );
void ClientSpawnBullet( Dictionary& params );
//--------------------------------------