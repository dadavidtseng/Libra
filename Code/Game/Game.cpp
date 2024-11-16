//----------------------------------------------------------------------------------------------------
// Game.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"

//----------------------------------------------------------------------------------------------------
class SpriteSheet;

//----------------------------------------------------------------------------------------------------
Game::Game()
{
    InitializeTiles();
    InitializeMaps();
    InitializeAudio();

    m_worldCamera  = new Camera();
    m_screenCamera = new Camera();

    m_playerTank = dynamic_cast<PlayerTank*>(m_currentMap->SpawnNewEntity(ENTITY_TYPE_PLAYER_TANK,
                                                                          ENTITY_FACTION_GOOD,
                                                                          Vec2(PLAYER_TANK_INIT_POSITION_X,
                                                                               PLAYER_TANK_INIT_POSITION_Y),
                                                                          PLAYER_TANK_INIT_ORIENTATION_DEGREES));

    Vec2 const bottomLeft = Vec2(0.f, 0.f);

    m_worldCamera->SetOrthoView(bottomLeft, Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
    m_screenCamera->SetOrthoView(bottomLeft, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));



    m_attractModePlayback = g_theAudio->StartSound(m_attractModeBgm, true, 1, 0, 1, false);
}

// void Game::ChangeCurrentMap( int newMapIndex)
// {
// 	m_currentMap->RemoveEntityFromMap(m_playerTank);
// 	m_currentMap = m_maps[newMapIndex];
// 	m_currentMap->AddEntityToMap(m_playerTank, Vec2(1.5f, 1.5f), 45.f);
// }

//----------------------------------------------------------------------------------------------------
Game::~Game()
{
    delete m_currentMap;
    m_currentMap = nullptr;

    m_maps.clear();

    delete m_playerTank;
    m_playerTank = nullptr;

    delete m_screenCamera;
    m_screenCamera = nullptr;

    delete m_worldCamera;
    m_worldCamera = nullptr;

    g_theAudio->StopSound(m_InGamePlayback);
    g_theAudio->StopSound(m_attractModePlayback);
};

//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
    // #TODO: Select keyboard or controller
    UpdateMarkForDelete();
    UpdateFromKeyBoard();
    UpdateFromController();
    UpdateCamera(deltaSeconds);
    UpdateAttractMode(deltaSeconds);
    AdjustForPauseAndTimeDistortion(deltaSeconds);

    m_currentMap->Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
    g_theRenderer->BeginCamera(*m_worldCamera);

    m_currentMap->Render();

    // TestTileDefinition();
    m_currentMap->DebugRender();

    g_theRenderer->EndCamera(*m_worldCamera);
    //-----------------------------------------------------------------------------------------------
    g_theRenderer->BeginCamera(*m_screenCamera);

    RenderAttractMode();

    RenderUI();

    g_theRenderer->EndCamera(*m_screenCamera);
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeMaps()
{
    MapData const data01 = { 0, IntVec2(m_mapDimension.x, m_mapDimension.y) };
    // MapData const data02 = { 1, IntVec2(0, 0) };
    // MapData const data03 = { 2, IntVec2(0, 0) };
    m_maps.reserve(3);
    m_maps.push_back(new Map(data01));
    // m_maps.push_back(new Map(data02));
    // m_maps.push_back(new Map(data03));
    m_currentMap = m_maps[0];
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeTiles()
{
    Texture*      tileTexture  = g_theRenderer->CreateOrGetTextureFromFile(TILE_TEXTURE_IMG);
    IntVec2 const spriteCoords = IntVec2(8, 8);
    m_tileSpriteSheet          = new SpriteSheet(*tileTexture, spriteCoords);

    TileDefinition::InitializeTileDefinitions(*m_tileSpriteSheet);
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeAudio()
{
    m_attractModeBgm = g_theAudio->CreateOrGetSound(ATTRACT_MODE_BGM);
    m_InGameBgm      = g_theAudio->CreateOrGetSound(IN_GAME_BGM);
    m_clickSound     = g_theAudio->CreateOrGetSound(CLICK_SOUND);
    m_pauseSound     = g_theAudio->CreateOrGetSound(PAUSE_SOUND);
    m_resumeSound    = g_theAudio->CreateOrGetSound(RESUME_SOUND);
}

//-----------------------------------------------------------------------------------------------
void Game::UpdateMarkForDelete()
{
    XboxController const& controller = g_theInput->GetController(0);

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) ||
        controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (m_isPaused &&
            !m_isAttractMode &&
            !m_isMarkedForDelete)
        {
            m_isMarkedForDelete = true;
            m_isAttractMode     = true;
            g_theAudio->StopSound(m_InGamePlayback);
            g_theAudio->StartSound(m_attractModeBgm);
            g_theAudio->StartSound(m_clickSound);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromKeyBoard()
{
    if (!m_isAttractMode &&
        g_theInput->WasKeyJustPressed(KEYCODE_F1))
        m_isDebugRendering = !m_isDebugRendering;

    if (!m_isAttractMode &&
        g_theInput->WasKeyJustPressed(KEYCODE_F4))
        m_isDebugCamera = !m_isDebugCamera;

    if (!m_isAttractMode &&
        g_theInput->WasKeyJustPressed(KEYCODE_F3))
        m_isNoClip = !m_isNoClip;


    if (g_theInput->WasKeyJustPressed(KEYCODE_P))
    {
        if (!m_isAttractMode)
        {
            if (!m_isPaused)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused)
            {
                m_isPaused = false;
                g_theAudio->StartSound(m_resumeSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
            }
        }

        if (m_isAttractMode)
        {
            m_isAttractMode = false;
            m_isPaused      = false;
            g_theAudio->StopSound(m_attractModePlayback);
            m_InGamePlayback = g_theAudio->StartSound(m_InGameBgm, true, 1, 0, m_InGameBgmSpeed, false);
            g_theAudio->StartSound(m_clickSound);
        }
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
    {
        if (!m_isPaused && !m_isMarkedForDelete && !m_isAttractMode)
        {
            m_isPaused = true;
            g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
        }
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_O))
    {
        if (!m_isAttractMode)
        {
            if (!m_isPaused)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused)
            {
                m_currentMap->Update(1.f / 60.f);
                m_playerTank->Update(1.f / 60.f);
            }
        }
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_T))
    {
        if (m_isAttractMode)
            return;

        m_isSlowMo = true;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.1f);
    }

    if (g_theInput->WasKeyJustReleased(KEYCODE_T))
    {
        if (m_isAttractMode)
            return;

        m_isSlowMo = false;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_Y))
    {
        if (m_isAttractMode)
            return;

        m_isFastMo = true;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 4.0f);
    }

    if (g_theInput->WasKeyJustReleased(KEYCODE_Y))
    {
        if (m_isAttractMode)
            return;

        m_isFastMo = false;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromController()
{
    XboxController const& controller = g_theInput->GetController(0);

    if (!m_isAttractMode &&
        controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP))
        m_isDebugRendering = !m_isDebugRendering;

    if (!m_isAttractMode &&
        controller.WasButtonJustPressed(XBOX_BUTTON_LSHOULDER))
        m_isDebugCamera = !m_isDebugCamera;

    if (!m_isAttractMode &&
        controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN))
        m_isNoClip = !m_isNoClip;

    if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
    {
        if (!m_isAttractMode)
        {
            if (!m_isPaused)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused)
            {
                m_isPaused = false;
                g_theAudio->StartSound(m_resumeSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
            }
        }

        if (m_isAttractMode)
        {
            m_isAttractMode = false;
            m_isPaused      = false;
            g_theAudio->StopSound(m_attractModePlayback);
            m_InGamePlayback = g_theAudio->StartSound(m_InGameBgm, true, 1, 0, m_InGameBgmSpeed, false);
            g_theAudio->StartSound(m_clickSound);
        }
    }

    if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (!m_isPaused && !m_isMarkedForDelete && !m_isAttractMode)
        {
            m_isPaused = true;
            g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
        }
    }

    if (controller.WasButtonJustPressed(XBOX_BUTTON_RSHOULDER))
    {
        if (!m_isAttractMode)
        {
            if (!m_isPaused)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused)
            {
                m_currentMap->Update(1.f / 60.f);
                m_playerTank->Update(1.f / 60.f);
            }
        }
    }

    if (controller.WasButtonJustPressed(XBOX_BUTTON_RTHUMB))
    {
        if (m_isAttractMode)
            return;

        m_isSlowMo = true;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.1f);
    }

    if (controller.WasButtonJustReleased(XBOX_BUTTON_RTHUMB))
    {
        if (m_isAttractMode)
            return;

        m_isSlowMo = false;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
    }

    if (controller.WasButtonJustPressed(XBOX_BUTTON_LTHUMB))
    {
        if (m_isAttractMode)
            return;

        m_isFastMo = true;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 4.0f);
    }

    if (controller.WasButtonJustReleased(XBOX_BUTTON_LTHUMB))
    {
        if (m_isAttractMode)
            return;

        m_isFastMo = false;
        g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateCamera(const float deltaSeconds) const
{
    UNUSED(deltaSeconds)

    if (!m_playerTank)
        return;

    const Vec2      playerTankPosition = m_playerTank->m_position;
    constexpr float mapMinX            = 0.f;
    constexpr float mapMinY            = 0.f;
    const float     mapMaxX            = static_cast<float>(m_mapDimension.x);
    const float     mapMaxY            = static_cast<float>(m_mapDimension.y);

    Vec2 cameraMin = Vec2(playerTankPosition.x - WORLD_CENTER_X, playerTankPosition.y - WORLD_CENTER_Y);
    Vec2 cameraMax = Vec2(playerTankPosition.x + WORLD_CENTER_X, playerTankPosition.y + WORLD_CENTER_Y);

    cameraMin.x = RangeMapClamped(cameraMin.x, mapMinX, mapMaxX - 16.f, mapMinX, mapMaxX - 16.f);
    cameraMax.x = RangeMapClamped(cameraMax.x, mapMinX + 16.f, mapMaxX, mapMinX + 16.f, mapMaxX);

    cameraMin.y = RangeMapClamped(cameraMin.y, mapMinY, mapMaxY - 8.f, mapMinY, mapMaxY - 8.f);
    cameraMax.y = RangeMapClamped(cameraMax.y, mapMinY + 8.f, mapMaxY, mapMinY + 8.f, mapMaxY);

    m_worldCamera->SetOrthoView(cameraMin, cameraMax);

    if (m_isDebugCamera)
    {
        const Vec2  bottomLeft = Vec2(0.f, 0.f);
        const float newScreenX = WORLD_SIZE_X * (static_cast<float>(m_mapDimension.y) / WORLD_SIZE_Y);
        const float newScreenY = static_cast<float>(m_mapDimension.y);

        m_worldCamera->SetOrthoView(bottomLeft, Vec2(newScreenX, newScreenY));
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateAttractMode(const float deltaSeconds)
{
    if (!m_isAttractMode)
        return;

    if (m_glowIncreasing)
    {
        m_glowIntensity += deltaSeconds;

        if (m_glowIntensity >= 1.f)
        {
            m_glowIntensity  = 1.f;
            m_glowIncreasing = false;
        }
    }
    else
    {
        m_glowIntensity -= deltaSeconds;

        if (m_glowIntensity <= 0.f)
        {
            m_glowIntensity  = 0.f;
            m_glowIncreasing = true;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::AdjustForPauseAndTimeDistortion(float& deltaSeconds) const
{
    if (m_isAttractMode)
        return;

    if (m_isPaused)
        deltaSeconds = 0.f;

    if (m_isSlowMo)
        deltaSeconds *= 1 / 10.f;

    if (m_isFastMo)
        deltaSeconds *= 4.f;
}

//----------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
    if (!m_isAttractMode)
        return;

    DebugDrawGlowCircle(Vec2(800, 400), 400.0f, DEBUG_RENDER_RED, m_glowIntensity);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
    if (m_isAttractMode)
        return;

    if (m_isPaused)
        DebugDrawGlowBox(Vec2(SCREEN_CENTER_X, SCREEN_CENTER_Y), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y), TRANSPARENT_BLACK,
                         1.f);
}
