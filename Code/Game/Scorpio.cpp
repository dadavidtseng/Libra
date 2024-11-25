//----------------------------------------------------------------------------------------------------
// Scorpio.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Scorpio.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"

//----------------------------------------------------------------------------------------------------
Scorpio::Scorpio(Map* map, EntityType const type, EntityFaction const faction)
    : Entity(map, type, faction)
{
    m_physicsRadius               = g_gameConfigBlackboard.GetValue("scorpioPhysicsRadius", 0.35f);
    m_detectRange                 = g_gameConfigBlackboard.GetValue("scorpioDetectRange", 10.f);
    m_isPushedByWalls             = g_gameConfigBlackboard.GetValue("scorpioIsPushedByWalls", true);
    m_isPushedByEntities          = g_gameConfigBlackboard.GetValue("scorpioIsPushedByEntities", false);
    m_doesPushEntities            = g_gameConfigBlackboard.GetValue("scorpioDoesPushEntities", true);
    m_health                      = g_gameConfigBlackboard.GetValue("scorpioInitHealth", 5);
    m_playerTankLastKnownPosition = m_position;
    
    m_bodyBounds    = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_turretBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_bodyTexture   = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_BODY_IMG);
    m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_TURRET_IMG);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::Update(float const deltaSeconds)
{
    if (m_isDead)
        return;

    if (g_theGame->GetPlayerTank()->m_isDead)
        return;

    if (m_health <= 0)
    {
        g_theAudio->StartSound(g_theGame->GetEnemyDiedSoundID());
        m_isGarbage = true;
        m_isDead    = true;
    }

    UpdateTurret(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::Render() const
{
    if (m_isDead)
        return;

    RenderBody();
    RenderTurret();
    RenderLaser();
}

//----------------------------------------------------------------------------------------------------
void Scorpio::DebugRender() const
{
    if (m_isDead)
        return;

    Vec2 const fwdNormal  = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
    Vec2 const leftNormal = fwdNormal.GetRotated90Degrees();

    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.03f,
                  Rgba8::CYAN);

    DebugDrawLine(m_position,
                  m_position + fwdNormal,
                  0.03f,
                  Rgba8::RED);
    DebugDrawLine(m_position,
                  m_position + leftNormal,
                  0.03f,
                  Rgba8::GREEN);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::UpdateTurret(float const deltaSeconds)
{
    if (m_shootCoolDown > 0.0f)
    {
        m_shootCoolDown -= deltaSeconds;
    }

    // Turn and shoot ( or turn idly)
    PlayerTank const* playerTank = g_theGame->GetPlayerTank();
    if (m_map->HasLineOfSight(m_position, playerTank->m_position, m_detectRange) && !playerTank->m_isDead)
    {
        // Turn toward player
        float const targetOrientationDegrees = (m_playerTankLastKnownPosition - m_position).GetOrientationDegrees();

        TurnToward(m_turretOrientationDegrees, targetOrientationDegrees, deltaSeconds, m_turretRotateSpeed);

        // Shot at player if facing close enough to orientation
        Vec2 const  dispToTarget    = playerTank->m_position - m_position;
        Vec2 const  myFwdNormal     = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
        float const degreesToTarget = GetAngleDegreesBetweenVectors2D(dispToTarget, myFwdNormal);

        if (degreesToTarget < m_shootDegreesThreshold &&
            m_shootCoolDown <= 0.0f)
        {
            m_map->SpawnNewEntity(ENTITY_TYPE_BULLET, ENTITY_FACTION_EVIL, m_position + myFwdNormal * 0.45f, m_turretOrientationDegrees);
            m_shootCoolDown = g_gameConfigBlackboard.GetValue("scorpioShootCoolDown", 0.3f);
            g_theAudio->StartSound(g_theGame->GetEnemyShootSoundID());
        }

        m_playerTankLastKnownPosition = playerTank->m_position;
    }
    else
    {
        // turn blindly
        m_turretOrientationDegrees += deltaSeconds * m_turretRotateSpeed;
    }
}

//----------------------------------------------------------------------------------------------------
void Scorpio::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_bodyBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.0f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_bodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}

//----------------------------------------------------------------------------------------------------
void Scorpio::RenderTurret() const
{
    std::vector<Vertex_PCU> turretVerts;
    AddVertsForAABB2D(turretVerts, m_turretBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(turretVerts.size()), turretVerts.data(),
                             1.0f, m_orientationDegrees + m_turretOrientationDegrees, m_position);

    g_theRenderer->BindTexture(m_turretTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(turretVerts.size()), turretVerts.data());
}

//----------------------------------------------------------------------------------------------------
void Scorpio::RenderLaser() const
{
    Vec2 const            fwdNormal       = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
    Ray2 const            ray             = Ray2(m_position, fwdNormal.GetNormalized(), 10000);
    RaycastResult2D const raycastResult2D = m_map->RaycastVsTiles(ray);

    DebugDrawLine(m_position + fwdNormal * 0.45f, raycastResult2D.m_impactPos, 0.05f, Rgba8::RED);
}
