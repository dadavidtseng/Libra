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


//-----------------------------------------------------------------------------------------------
// Scorpio-related
//
constexpr float SCORPIO_ANGULAR_VELOCITY        = 90.f;
constexpr float SCORPIO_PHYSICS_RADIUS          = 0.35f;
constexpr float SCORPIO_SHOOT_DEGREES_THRESHOLD = 5.f;
constexpr float SCORPIO_RANGE                   = 2.f;
constexpr float SCORPIO_SHOOT_COOLDOWN          = 0.3f;

//-----------------------------------------------------------------------------------------------
// Leo-related
//
constexpr float LEO_SPEED            = 0.5f;
constexpr float LEO_ANGULAR_VELOCITY = 90.f;
constexpr float LEO_PHYSICS_RADIUS   = 0.35f;
constexpr float LEO_RANGE            = 10.f;
constexpr float LEO_SHOOT_COOLDOWN   = 1.f;

//-----------------------------------------------------------------------------------------------
// Aries-related
//

//-----------------------------------------------------------------------------------------------
// Bullet-related
//
constexpr float BULLET_EVIL_SPEED = 3.f;

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
extern const char* CLICK_SOUND;
extern const char* PAUSE_SOUND;
extern const char* RESUME_SOUND;

//-----------------------------------------------------------------------------------------------
// Texture-related
//
extern const char* PLAYER_TANK_BODY_IMG;
extern const char* PLAYER_TANK_TURRET_IMG;
extern const char* ENEMY_TANK_SCORPIO_BODY_IMG;
extern const char* ENEMY_TANK_SCORPIO_TURRET_IMG;
extern const char* ENEMY_TANK_LEO_BODY_IMG;
extern const char* ENEMY_TANK_ARIES_BODY_IMG;
extern const char* BULLET_GOOD_IMG;
extern const char* BULLET_EVIL_IMG;
extern const char* TILE_TEXTURE_IMG;
