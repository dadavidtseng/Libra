//-----------------------------------------------------------------------------------------------
// GameCommon.hpp
//

//-----------------------------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
class App;
class AudioSystem;
class Game;
class InputSystem;
class Renderer;
class RandomNumberGenerator;

// one-time declaration
extern App*                   g_theApp;
extern AudioSystem*           g_theAudio;
extern Game*                  g_theGame;
extern InputSystem*           g_theInput;
extern Renderer*              g_theRenderer;
extern RandomNumberGenerator* g_theRNG;

//-----------------------------------------------------------------------------------------------
// initial settings
//
constexpr float SCREEN_SIZE_X   = 1600.f;
constexpr float SCREEN_SIZE_Y   = 800.f;
constexpr float WORLD_SIZE_X    = 16.f;
constexpr float WORLD_SIZE_Y    = 8.f;
constexpr float WORLD_CENTER_X  = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y  = WORLD_SIZE_Y / 2.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;

//-----------------------------------------------------------------------------------------------
// PlayerTank-related
//
constexpr float PLAYER_TANK_INIT_POSITION_X          = 2.f;
constexpr float PLAYER_TANK_INIT_POSITION_Y          = 2.f;
constexpr float PLAYER_TANK_INIT_ORIENTATION_DEGREES = 30.f;
constexpr float PLAYER_TANK_PHYSICS_RADIUS           = 0.3f;
constexpr float PLAYER_TANK_ANGULAR_VELOCITY         = 180.f;
constexpr float PLAYER_TANK_SHOOT_COOLDOWN           = 0.1f;
constexpr int   PLAYER_TANK_INIT_HEALTH              = 10;

//-----------------------------------------------------------------------------------------------
// Scorpio-related
//
constexpr float SCORPIO_ANGULAR_VELOCITY        = 90.f;
constexpr float SCORPIO_PHYSICS_RADIUS          = 0.35f;
constexpr float SCORPIO_SHOOT_DEGREES_THRESHOLD = 5.f;
constexpr float SCORPIO_RANGE                   = 10.f;
constexpr float SCORPIO_SHOOT_COOLDOWN          = 0.3f;
constexpr int   SCORPIO_INIT_HEALTH             = 5;

//-----------------------------------------------------------------------------------------------
// Leo-related
//
constexpr float LEO_MOVE_SPEED       = 0.5f;
constexpr float LEO_ANGULAR_VELOCITY = 90.f;
constexpr float LEO_PHYSICS_RADIUS   = 0.25f;
constexpr float LEO_RANGE            = 10.f;
constexpr float LEO_SHOOT_COOLDOWN   = 1.f;
constexpr int   LEO_INIT_HEALTH      = 3;

//-----------------------------------------------------------------------------------------------
// Aries-related
//
constexpr float ARIES_MOVE_SPEED       = 0.5f;
constexpr float ARIES_ANGULAR_VELOCITY = 90.f;
constexpr float ARIES_PHYSICS_RADIUS   = 0.25f;
constexpr float ARIES_RANGE            = 10.f;
constexpr int   ARIES_INIT_HEALTH      = 8;

//-----------------------------------------------------------------------------------------------
// Bullet-related
//
constexpr float BULLET_GOOD_SPEED       = 5.f;
constexpr float BULLET_EVIL_SPEED       = 3.f;
constexpr int   BULLET_GOOD_INIT_HEALTH = 3;
constexpr int   BULLET_EVIL_INIT_HEALTH = 1;


//-----------------------------------------------------------------------------------------------
// DebugRender-related
//
extern Rgba8 const DEBUG_RENDER_GREY;
extern Rgba8 const DEBUG_RENDER_RED;
extern Rgba8 const DEBUG_RENDER_GREEN;
extern Rgba8 const DEBUG_RENDER_MAGENTA;
extern Rgba8 const DEBUG_RENDER_CYAN;
extern Rgba8 const DEBUG_RENDER_YELLOW;
extern Rgba8 const DEBUG_RENDER_BLUE;
extern Rgba8 const TRANSPARENT_BLACK;
extern Rgba8 const TRANSPARENT_RED;
extern Rgba8 const TRANSPARENT_GREEN;
extern Rgba8 const BLACK;

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void DebugDrawGlowCircle(Vec2 const& center, float radius, Rgba8 const& color, float glowIntensity);
void DebugDrawGlowBox(Vec2 const& center, Vec2 const& dimensions, Rgba8 const& color, float glowIntensity);
void DebugDrawBoxRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);

//-----------------------------------------------------------------------------------------------
// Audio-related
//
extern const char* ATTRACT_MODE_BGM;
extern const char* IN_GAME_BGM;
extern const char* GAME_WIN_BGM;
extern const char* GAME_LOSE_BGM;
extern const char* CLICK_SOUND;
extern const char* PAUSE_SOUND;
extern const char* RESUME_SOUND;
extern const char* PLAYER_TANK_SHOOT_SOUND;
extern const char* PLAYER_TANK_HIT_SOUND;
extern const char* ENEMY_DIED_SOUND;
extern const char* ENEMY_HIT_SOUND;
extern const char* ENEMY_SHOOT_SOUND;
extern const char* EXIT_MAP_SOUND;
extern const char* BULLET_BOUNCE_SOUND;

//-----------------------------------------------------------------------------------------------
// Texture-related
//
extern const char* PLAYER_TANK_BODY_IMG;
extern const char* PLAYER_TANK_TURRET_IMG;
extern const char* SCORPIO_BODY_IMG;
extern const char* SCORPIO_TURRET_IMG;
extern const char* LEO_BODY_IMG;
extern const char* ARIES_BODY_IMG;
extern const char* BULLET_GOOD_IMG;
extern const char* BULLET_EVIL_IMG;
extern const char* TILE_TEXTURE_IMG;
