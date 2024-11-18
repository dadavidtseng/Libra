//-----------------------------------------------------------------------------------------------
// Game.hpp
//

//-----------------------------------------------------------------------------------------------
#pragma once
#include "TileDefinition.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

//-----------------------------------------------------------------------------------------------
class Camera;
class Map;
class PlayerTank;

//-----------------------------------------------------------------------------------------------
class Game
{
public:
    Game();
    ~Game();

    void         Update(float deltaSeconds);
    void         Render() const;
    bool         IsAttractMode() const { return m_isAttractMode; }
    bool         IsNoClip() const { return m_isNoClip; }
    bool         IsDebugRendering() const { return m_isDebugRendering; }
    bool         IsMarkedForDelete() const { return m_isMarkedForDelete; }
    PlayerTank*  GetPlayerTank() const { return m_playerTank; }
    SpriteSheet* GetTileSpriteSheet() const { return m_tileSpriteSheet; }

private:
    void InitializeMaps();
    void InitializeTiles();
    void InitializeAudio();

    void UpdateMarkForDelete();
    void UpdateFromKeyBoard();
    void UpdateFromController();
    void UpdateCurrentMap();
    void UpdateCamera(float deltaSeconds) const;
    void UpdateAttractMode(float deltaSeconds);
    void AdjustForPauseAndTimeDistortion(float& deltaSeconds) const;

    void RenderAttractMode() const;
    void RenderUI() const;


    Camera* m_worldCamera  = nullptr;
    Camera* m_screenCamera = nullptr;
    // IntVec2 m_mapDimension      = IntVec2(24, 30);
    bool  m_isAttractMode     = true;
    bool  m_isGameOverMode    = false;
    bool  m_isDebugRendering  = false;
    bool  m_isDebugCamera     = false;
    bool  m_isPaused          = false;
    bool  m_isSlowMo          = false;
    bool  m_isFastMo          = false;
    bool  m_isMarkedForDelete = false;
    bool  m_isNoClip          = false;
    float m_glowIntensity     = 0.f;
    float m_gameOverCountDown = 0.f;
    bool  m_glowIncreasing    = false;
    Vec2  m_baseCameraPos     = Vec2(0.f, 0.f);

    std::vector<Map*> m_maps;
    Map*              m_currentMap      = nullptr;
    SpriteSheet*      m_tileSpriteSheet = nullptr;
    PlayerTank*       m_playerTank      = nullptr;

    SoundID         m_attractModeBgm      = 0;
    SoundPlaybackID m_attractModePlayback = 0;
    SoundID         m_InGameBgm           = 0;
    SoundPlaybackID m_InGamePlayback      = 0;
    SoundID         m_clickSound          = 0;
    SoundID         m_pauseSound          = 0;
    SoundID         m_resumeSound         = 0;
    float           m_InGameBgmSpeed      = 1.f;

    // float numTilesInViewHorizontally
};
