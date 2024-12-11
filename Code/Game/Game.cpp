//----------------------------------------------------------------------------------------------------
// Game.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"



//----------------------------------------------------------------------------------------------------
Game::Game()
{
    InitializeTiles();
    InitializeMaps();
    InitializeAudio();

    Texture const* const tileTexture  = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestSpriteSheet_8x2.png");
    IntVec2 const        spriteCoords = IntVec2(8, 2);
    m_testSpriteSheet                 = new SpriteSheet(*tileTexture, spriteCoords);

    m_worldCamera  = new Camera();
    m_screenCamera = new Camera();

    Vec2 const  playerTankInitPosition           = g_gameConfigBlackboard.GetValue("playerTankInitPosition", Vec2(2.f, 2.f));
    float const playerTankInitOrientationDegrees = g_gameConfigBlackboard.GetValue("playerTankInitOrientationDegrees", 30.f);



    m_playerTank = dynamic_cast<PlayerTank*>(m_currentMap->SpawnNewEntity(ENTITY_TYPE_PLAYER_TANK,
                                                                          ENTITY_FACTION_GOOD,
                                                                          playerTankInitPosition,
                                                                          playerTankInitOrientationDegrees));

    Vec2 const  bottomLeft  = Vec2::ZERO;
    float const worldSizeX  = g_gameConfigBlackboard.GetValue("worldSizeX", 16.f);
    float const worldSizeY  = g_gameConfigBlackboard.GetValue("worldSizeY", 8.f);
    float const screenSizeX = g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f);
    float const screenSizeY = g_gameConfigBlackboard.GetValue("screenSizeY", 800.f);

    m_worldCamera->SetOrthoView(bottomLeft, Vec2(worldSizeX, worldSizeY));
    m_screenCamera->SetOrthoView(bottomLeft, Vec2(screenSizeX, screenSizeY));

    m_attractModePlayback = g_theAudio->StartSound(m_attractModeBgm, true, 3, 0, 1, false);
}

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
    g_theAudio->StopSound(m_gameWinPlayback);
    g_theAudio->StopSound(m_gameLosePlayback);
}

;

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

    m_testSecond += deltaSeconds;

    if (m_playerTank->m_isDead)
    {
        m_gameOverCountDown -= deltaSeconds;
    }

    if (m_playerTank->m_isDead &&
        m_gameOverCountDown <= 0.f)
    {
        m_isPaused          = true;
        m_isGameLoseMode    = true;
        m_gameOverCountDown = 3.f;
        g_theAudio->StopSound(m_InGamePlayback);

        if (m_gameLosePlayback == 0)
            m_gameLosePlayback = g_theAudio->StartSound(m_gameLoseBgm);
    }

    if (m_currentMap->GetTileCoordsFromWorldPos(m_playerTank->m_position).x == m_currentMap->GetMapExitPosition().x &&
        m_currentMap->GetTileCoordsFromWorldPos(m_playerTank->m_position).y == m_currentMap->GetMapExitPosition().y)
    {
        if (m_currentMap->GetMapIndex() == 2)
        {
            m_isPaused      = true;
            m_isGameWinMode = true;
            g_theAudio->StopSound(m_InGamePlayback);

            if (m_gameWinPlayback == 0)
                m_gameWinPlayback = g_theAudio->StartSound(m_gameWinBgm);

            return;
        }

        UpdateCurrentMap();
    }

    if (m_isUpdateMapCountingDown)
    {
        m_updateMapCountDown -= deltaSeconds;

        if (m_updateMapCountDown <= 0)
        {
            UpdateCurrentMap();
            m_updateMapCountDown      = 1.f;
            m_isUpdateMapCountingDown = false;
            return;
        }
    }


    if (m_currentMap)
        m_currentMap->Update(deltaSeconds);

    if (g_theInput->WasKeyJustPressed(KEYCODE_F))
    {
        g_theDevConsole->ToggleMode(OPEN_FULL);
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
    {
        g_theDevConsole->AddLine(Rgba8::WHITE, "SHOOT");
        g_theDevConsole->Execute("SHOOT");
        // EventArgs args;
        // args.SetValue("SHOOT", "SHOOT");
        // g_theEventSystem->FireEvent("SHOOT", args);
    }
}

void Game::TestBitfontMap() const
{
    if (!m_isAttractMode)
        return;

    std::vector<Vertex_PCU> textVerts;
    g_theBitmapFont->AddVertsForText2D(textVerts, Vec2(100.f, 200.f), 30.f, "Hello, world");
    g_theBitmapFont->AddVertsForText2D(textVerts, Vec2(20.f, 300.f), 30.f, std::to_string(m_testSecond));
    g_theBitmapFont->AddVertsForText2D(textVerts, Vec2(250.f, 400.f), 15.f, "It's nice to have options!", Rgba8::RED, 0.6f);
    g_theRenderer->BindTexture(&g_theBitmapFont->GetTexture());
    g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
}
void Game::TestTextBox2D() const
{
    if (!m_isAttractMode)
        return;

    AABB2 box(Vec2(400.f, 400.f), Vec2(700.f, 700.f)); // 200x100 的邊界框

    VertexList boxVerts;
    AddVertsForAABB2D(boxVerts, box, Rgba8::BLUE);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(static_cast<int>(boxVerts.size()), boxVerts.data());

    VertexList vertexArray;

    // 設置文字與邊界框
    String text = "Hello, World!\nThis is multi-line text.\nThis is the third line TEST\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX\nX";

    float cellHeight = 20.f; // 每個字元高度
    Vec2  alignment(0.f, 0.f); // 中心對齊
    UNUSED(cellHeight)
    // 呼叫 AddVertsForTextInBox2D
    g_theBitmapFont->AddVertsForTextInBox2D(
        vertexArray,    // 頂點數據
        text,           // 文字內容
        box,            // 邊界框
        m_glowIntensity * 30,     // 字元高度
        Rgba8::WHITE,   // 字體顏色
        m_glowIntensity * 2,            // 寬高比例
        alignment,      // 對齊方式
        SHRINK_TO_FIT // 繪製模式


    );

    g_theRenderer->BindTexture(&g_theBitmapFont->GetTexture());
    g_theRenderer->DrawVertexArray(static_cast<int>(vertexArray.size()), vertexArray.data());
}
void Game::TestSpriteAnim() const
{
    if (!m_isAttractMode)
        return;

    VertexList vertexArray;

    // 創建動畫定義，從第0到第5幀，每秒10幀，循環播放
    SpriteAnimDefinition myAnim(*m_testSpriteSheet, 0, 5, 10.f, SpriteAnimPlaybackType::PINGPONG);

    // 假設每次調用此函數的時候我們傳入的時間為 elapsedTime

    // 根據動畫時間取得對應的 SpriteDefinition
    const SpriteDefinition& spriteDef = myAnim.GetSpriteDefAtTime(m_testSecond / 10);

    // 創建一個矩形範圍來渲染當前幀
    Vec2 mins(0.f, 0.f);  // 在這裡可以調整你的位置或大小
    Vec2 maxs(1.f, 1.f);   // 同樣也可以改變這些值來縮放精靈

    Vec2 uvMins = spriteDef.GetUVsMins();
    Vec2 uvMaxs = spriteDef.GetUVsMaxs();
    // printf("( %f, %f ) ( %f, %f )\n", uvMins.x, uvMins.y, uvMaxs.x, uvMaxs.y);
    // 添加頂點
    AddVertsForAABB2D(vertexArray, AABB2(mins, maxs), Rgba8::WHITE, uvMins, uvMaxs);


    // 綁定精靈紋理
    g_theRenderer->BindTexture(&spriteDef.GetTexture());

    // 繪製動畫幀
    g_theRenderer->DrawVertexArray(static_cast<int>(vertexArray.size()), vertexArray.data());

}
void Game::TestDevConsole() const
{
    AABB2 box = AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 100.f));

    g_theDevConsole->Render(box);
}

//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
    g_theRenderer->BeginCamera(*m_worldCamera);

    m_currentMap->Render();
    m_currentMap->DebugRender();
    TestSpriteAnim();


    g_theRenderer->EndCamera(*m_worldCamera);
    //-----------------------------------------------------------------------------------------------
    g_theRenderer->BeginCamera(*m_screenCamera);

    RenderAttractMode();
    RenderUI();
    TestBitfontMap();
    TestTextBox2D();
    TestDevConsole();
    m_currentMap->RenderTileHeatMapText();



    g_theRenderer->EndCamera(*m_screenCamera);
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeMaps()
{
    printf("( Game ) Start  | InitializeMaps\n");

    MapDefinition::InitializeMapDefs();

    m_maps.reserve(3);

    for (int mapIndex = 0; mapIndex < 3; ++mapIndex)
    {
        m_maps.push_back(new Map(*MapDefinition::s_mapDefinitions[mapIndex]));
    }

    m_currentMap = m_maps[0];

    printf("( Game ) Finish | InitializeMaps\n");
}


//----------------------------------------------------------------------------------------------------
void Game::InitializeTiles()
{
    printf("( Game ) Start  | InitializeTiles\n");

    Texture const* const tileTexture  = g_theRenderer->CreateOrGetTextureFromFile(TILE_TEXTURE_IMG);
    IntVec2 const        spriteCoords = IntVec2(8, 8);
    m_tileSpriteSheet                 = new SpriteSheet(*tileTexture, spriteCoords);

    TileDefinition::InitializeTileDefs(*m_tileSpriteSheet);

    printf("( Game ) Finish | InitializeTiles\n");
}

//----------------------------------------------------------------------------------------------------
void Game::InitializeAudio()
{
    printf("( Game ) Start  | InitializeAudio\n");

    m_attractModeBgm       = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("attractModeBgm", "Data/Audios/AttractModeBgm.mp3"));
    m_InGameBgm            = g_theAudio->CreateOrGetSound(IN_GAME_BGM);
    m_gameWinBgm           = g_theAudio->CreateOrGetSound(GAME_WIN_BGM);
    m_gameLoseBgm          = g_theAudio->CreateOrGetSound(GAME_LOSE_BGM);
    m_clickSound           = g_theAudio->CreateOrGetSound(CLICK_SOUND);
    m_pauseSound           = g_theAudio->CreateOrGetSound(PAUSE_SOUND);
    m_resumeSound          = g_theAudio->CreateOrGetSound(RESUME_SOUND);
    m_playerTankShootSound = g_theAudio->CreateOrGetSound(PLAYER_TANK_SHOOT_SOUND);
    m_playerTankHitSound   = g_theAudio->CreateOrGetSound(PLAYER_TANK_HIT_SOUND);
    m_enemyDiedSound       = g_theAudio->CreateOrGetSound(ENEMY_DIED_SOUND);
    m_enemyHitSound        = g_theAudio->CreateOrGetSound(ENEMY_HIT_SOUND);
    m_enemyShootSound      = g_theAudio->CreateOrGetSound(ENEMY_SHOOT_SOUND);
    m_exitMapSound         = g_theAudio->CreateOrGetSound(EXIT_MAP_SOUND);
    m_bulletBounceSound    = g_theAudio->CreateOrGetSound(BULLET_BOUNCE_SOUND);
    m_enemyDiscoverSound = g_theAudio->CreateOrGetSound(ENEMY_DISCOVER_SOUND);

    printf("( Game ) Finish | InitializeAudio\n");
}

//-----------------------------------------------------------------------------------------------
void Game::UpdateMarkForDelete()
{
    XboxController const& controller = g_theInput->GetController(0);

    if (m_isPaused &&
        !m_isMarkedForDelete &&
        !m_isAttractMode)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) ||
            controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            m_isMarkedForDelete = true;
            m_isAttractMode     = true;
            g_theAudio->StopSound(m_InGamePlayback);
            g_theAudio->StartSound(m_clickSound);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromKeyBoard()
{
    // AttractMode
    if (m_isAttractMode)
    {
        if (g_theInput->WasKeyJustPressed(KEYCODE_P))
        {
            m_isAttractMode = false;
            m_isPaused      = false;
            g_theAudio->StopSound(m_attractModePlayback);
            m_InGamePlayback = g_theAudio->StartSound(m_InGameBgm, true, 1, 0, m_InGameBgmSpeed, false);
            g_theAudio->StartSound(m_clickSound);
            return;
        }
    }

    // InGameMode
    if (!m_isAttractMode)
    {
        // GameWinMode
        if (m_isGameWinMode)
        {
            if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
            {
                m_isGameWinMode     = false;
                m_isMarkedForDelete = true;
                g_theAudio->StopSound(m_gameWinPlayback);
                g_theAudio->StartSound(m_attractModeBgm);
                g_theAudio->StartSound(m_clickSound);
            }
        }

        // GameLoseMode
        if (m_isGameLoseMode)
        {
            if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
            {
                m_isGameLoseMode    = false;
                m_isMarkedForDelete = true;
                g_theAudio->StopSound(m_gameLosePlayback);
                g_theAudio->StartSound(m_attractModeBgm);
                g_theAudio->StartSound(m_clickSound);
            }

            if (g_theInput->WasKeyJustPressed(KEYCODE_N))
            {
                m_playerTank->m_health = g_gameConfigBlackboard.GetValue("playerTankInitHealth", 100);
                m_playerTank->m_isDead = false;
                m_isGameLoseMode       = false;
                m_isPaused             = false;
            }
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
        {
            m_isDebugRendering = !m_isDebugRendering;
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
        {
            m_isNoClip = !m_isNoClip;
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
        {
            m_isDebugCamera = !m_isDebugCamera;
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
        {
            if (m_currentMap->GetMapIndex() == 2)
            {
                m_isPaused      = true;
                m_isGameWinMode = true;
                g_theAudio->StopSound(m_InGamePlayback);
                m_gameWinPlayback = g_theAudio->StartSound(m_gameWinBgm);

                return;
            }

            // UpdateCurrentMap();
            m_isUpdateMapCountingDown = true;
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_P))
        {
            if (!m_isPaused && !m_isGameWinMode && !m_isGameLoseMode)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused && !m_isGameWinMode && !m_isGameLoseMode)
            {
                m_isPaused = false;
                g_theAudio->StartSound(m_resumeSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
            }
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_O))
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

        if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
        {
            if (!m_isPaused && !m_isMarkedForDelete)
            {
                if (m_isGameWinMode)
                    return;
                if (m_isGameLoseMode)
                    return;

                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_T))
        {
            m_isSlowMo = true;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.1f);
        }

        if (g_theInput->WasKeyJustReleased(KEYCODE_T))
        {
            m_isSlowMo = false;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
        }

        if (g_theInput->WasKeyJustPressed(KEYCODE_Y))
        {
            m_isFastMo = true;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 4.0f);
        }

        if (g_theInput->WasKeyJustReleased(KEYCODE_Y))
        {
            m_isFastMo = false;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateFromController()
{
    XboxController const& controller = g_theInput->GetController(0);

    // AttractMode
    if (m_isAttractMode)
    {
        if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
        {
            m_isAttractMode = false;
            m_isPaused      = false;
            g_theAudio->StopSound(m_attractModePlayback);
            m_InGamePlayback = g_theAudio->StartSound(m_InGameBgm, true, 1, 0, m_InGameBgmSpeed, false);
            g_theAudio->StartSound(m_clickSound);
            return;
        }
    }

    // InGameMode
    if (!m_isAttractMode)
    {
        // GameWinMode
        if (m_isGameWinMode)
        {
            if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
            {
                m_isGameWinMode     = false;
                m_isMarkedForDelete = true;
                g_theAudio->StopSound(m_gameWinPlayback);
                g_theAudio->StartSound(m_attractModeBgm);
                g_theAudio->StartSound(m_clickSound);
            }
        }

        // GameLoseMode
        if (m_isGameLoseMode)
        {
            if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
            {
                m_isGameLoseMode    = false;
                m_isMarkedForDelete = true;
                g_theAudio->StopSound(m_gameLosePlayback);
                g_theAudio->StartSound(m_attractModeBgm);
                g_theAudio->StartSound(m_clickSound);
            }

            if (controller.WasButtonJustPressed(XBOX_BUTTON_A))
            {
                m_playerTank->m_health = g_gameConfigBlackboard.GetValue("playerTankInitHealth", 100);
                m_playerTank->m_isDead = false;
                m_isGameLoseMode       = false;
                m_isPaused             = false;
            }
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP))
        {
            m_isDebugRendering = !m_isDebugRendering;
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN))
        {
            m_isNoClip = !m_isNoClip;
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_LSHOULDER))
        {
            m_isDebugCamera = !m_isDebugCamera;
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_B))
        {
            if (m_currentMap->GetMapIndex() == 2)
            {
                m_isPaused      = true;
                m_isGameWinMode = true;
                g_theAudio->StopSound(m_InGamePlayback);
                m_gameWinPlayback = g_theAudio->StartSound(m_gameWinBgm);

                return;
            }

            UpdateCurrentMap();
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_START))
        {
            if (!m_isPaused && !m_isGameWinMode && !m_isGameLoseMode)
            {
                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
            else if (m_isPaused && !m_isGameWinMode && !m_isGameLoseMode)
            {
                m_isPaused = false;
                g_theAudio->StartSound(m_resumeSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
            }
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_RSHOULDER))
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

        if (controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
        {
            if (!m_isPaused && !m_isMarkedForDelete)
            {
                if (m_isGameWinMode)
                    return;
                if (m_isGameLoseMode)
                    return;

                m_isPaused = true;
                g_theAudio->StartSound(m_pauseSound, false, 1, 0, 1, false);
                g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.f);
            }
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_RTHUMB))
        {
            m_isSlowMo = true;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 0.1f);
        }

        if (controller.WasButtonJustReleased(XBOX_BUTTON_RTHUMB))
        {
            m_isSlowMo = false;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_LTHUMB))
        {
            m_isFastMo = true;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 4.0f);
        }

        if (controller.WasButtonJustReleased(XBOX_BUTTON_LTHUMB))
        {
            m_isFastMo = false;
            g_theAudio->SetSoundPlaybackSpeed(m_InGamePlayback, 1.f);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateCurrentMap()
{
    int const   currentMapIndex                  = m_currentMap->GetMapIndex();
    Vec2 const  playerTankInitPosition           = g_gameConfigBlackboard.GetValue("playerTankInitPosition", Vec2(2.f, 2.f));
    float const playerTankInitOrientationDegrees = g_gameConfigBlackboard.GetValue("playerTankInitOrientationDegrees", 30.f);

    m_currentMap->RemoveEntityFromMap(m_playerTank);
    m_currentMap = m_maps[currentMapIndex + 1];
    m_currentMap->AddEntityToMap(m_playerTank,
                                 playerTankInitPosition,
                                 playerTankInitOrientationDegrees);
    m_playerTank->SetBodyScale(0);

    g_theAudio->StartSound(m_exitMapSound);
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateCamera(float const deltaSeconds) const
{
    UNUSED(deltaSeconds)

    float const worldSizeX   = g_gameConfigBlackboard.GetValue("worldSizeX", 16.f);
    float const worldSizeY   = g_gameConfigBlackboard.GetValue("worldSizeY", 8.f);
    float const worldCenterX = g_gameConfigBlackboard.GetValue("worldCenterX", 8.f);
    float const worldCenterY = g_gameConfigBlackboard.GetValue("worldCenterY", 4.f);

    if (!m_playerTank)
        return;

    const Vec2      playerTankPosition = m_playerTank->m_position;
    constexpr float mapMinX            = 0.f;
    constexpr float mapMinY            = 0.f;
    const float     mapMaxX            = static_cast<float>(m_currentMap->GetMapDimension().x);
    const float     mapMaxY            = static_cast<float>(m_currentMap->GetMapDimension().y);

    Vec2 cameraMin = Vec2(playerTankPosition.x - worldCenterX, playerTankPosition.y - worldCenterY);
    Vec2 cameraMax = Vec2(playerTankPosition.x + worldCenterX, playerTankPosition.y + worldCenterY);

    cameraMin.x = RangeMapClamped(cameraMin.x, mapMinX, mapMaxX - worldSizeX, mapMinX, mapMaxX - worldSizeX);
    cameraMax.x = RangeMapClamped(cameraMax.x, mapMinX + worldSizeX, mapMaxX, mapMinX + worldSizeX, mapMaxX);

    cameraMin.y = RangeMapClamped(cameraMin.y, mapMinY, mapMaxY - worldSizeY, mapMinY, mapMaxY - worldSizeY);
    cameraMax.y = RangeMapClamped(cameraMax.y, mapMinY + worldSizeY, mapMaxY, mapMinY + worldSizeY, mapMaxY);

    m_worldCamera->SetOrthoView(cameraMin, cameraMax);

    if (m_isDebugCamera)
    {
        Vec2 const bottomLeft = Vec2::ZERO;

        float const mapWidth  = static_cast<float>(m_currentMap->GetMapDimension().x);
        float const mapHeight = static_cast<float>(m_currentMap->GetMapDimension().y);

        float aspectRatio = worldSizeX / worldSizeY;
        float newScreenX,newScreenY;

        if (mapWidth / mapHeight > aspectRatio)
        {
            newScreenX = mapWidth;
            newScreenY = mapWidth / aspectRatio;
        }
        else
        {
            newScreenX = mapHeight * aspectRatio;
            newScreenY = mapHeight;
        }

        m_worldCamera->SetOrthoView(bottomLeft, Vec2(newScreenX, newScreenY));
    }
}

//----------------------------------------------------------------------------------------------------
void Game::UpdateAttractMode(float const deltaSeconds)
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

    DebugDrawGlowCircle(Vec2(800, 400),
                        400.0f,
                        Rgba8::RED,
                        m_glowIntensity);
}

//----------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
    float const screenSizeX   = g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f);
    float const screenSizeY   = g_gameConfigBlackboard.GetValue("screenSizeY", 800.f);
    float const screenCenterX = g_gameConfigBlackboard.GetValue("screenCenterX", 800.f);
    float const screenCenterY = g_gameConfigBlackboard.GetValue("screenCenterY", 400.f);

    if (m_isAttractMode)
        return;

    if (m_isPaused)
    {
        DebugDrawGlowBox(Vec2(screenCenterX, screenCenterY),
                         Vec2(screenSizeX, screenSizeY), Rgba8::BLACK,
                         0.5f);
    }

    if (m_isGameLoseMode)
    {
        DebugDrawGlowBox(Vec2(screenCenterX, screenCenterY),
                         Vec2(screenSizeX, screenSizeY),
                         Rgba8::RED,
                         0.5f);

        std::vector<Vertex_PCU> titleVerts;

        AddVertsForTextTriangles2D(titleVerts,
                                   "You are dead...",
                                   Vec2(30.f, screenCenterY + 100.f),
                                   48.f,
                                   Rgba8::BLACK,
                                   1.f,
                                   true,
                                   0.05f);

        AddVertsForTextTriangles2D(titleVerts,
                                   "Press \"N\" to respawn,",
                                   Vec2(30.f, screenCenterY),
                                   48.f,
                                   Rgba8::BLACK,
                                   1.f,
                                   true,
                                   0.05f);

        AddVertsForTextTriangles2D(titleVerts,
                                   "Press \"ESC\" to exit,",
                                   Vec2(30.f, screenCenterY - 100.f),
                                   48.f,
                                   Rgba8::BLACK,
                                   1.f,
                                   true,
                                   0.05f);

        g_theRenderer->DrawVertexArray(static_cast<int>(titleVerts.size()), titleVerts.data());
    }

    if (m_isGameWinMode)
    {
        DebugDrawGlowBox(Vec2(screenCenterX, screenCenterY),
                         Vec2(screenSizeX, screenSizeY),
                         Rgba8::GREEN,
                         0.5f);

        std::vector<Vertex_PCU> titleVerts;

        AddVertsForTextTriangles2D(titleVerts,
                                   "Victory!",
                                   Vec2(30.f, screenCenterY + 100.f),
                                   48.f,
                                   Rgba8::BLACK,
                                   1.f,
                                   true,
                                   0.05f);

        AddVertsForTextTriangles2D(titleVerts,
                                   "Press \"ESC\" to exit,",
                                   Vec2(30.f, screenCenterY - 100.f),
                                   48.f,
                                   Rgba8::BLACK,
                                   1.f,
                                   true,
                                   0.05f);

        g_theRenderer->DrawVertexArray(static_cast<int>(titleVerts.size()), titleVerts.data());
    }
}
