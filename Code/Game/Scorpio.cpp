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
    m_physicsRadius               = SCORPIO_PHYSICS_RADIUS;
    m_playerTankLastKnownPosition = m_position;

    m_isPushedByWalls    = true;
    m_isPushedByEntities = false;
    m_doesPushEntities   = true;

    m_health = SCORPIO_INIT_HEALTH;

    m_bodyBounds    = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_turretBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_bodyTexture   = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_BODY_IMG);
    m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_TURRET_IMG);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::Update(float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    if (m_isDead)
        return;

    if (g_theGame->GetPlayerTank()->m_isDead)
        return;

    if (m_health <= 0)
    {
        m_isGarbage = true;
        m_isDead    = true;
    }

    UpdateTurret(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (m_isDead)
        return;

    RenderBody();
    RenderTurret();
    RenderLaser();
}

//----------------------------------------------------------------------------------------------------
void Scorpio::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;

    if (m_isDead)
        return;

    Vec2 const fwdNormal  = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
    Vec2 const leftNormal = fwdNormal.GetRotated90Degrees();

    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.03f,
                  DEBUG_RENDER_CYAN);

    DebugDrawLine(m_position,
                  m_position + fwdNormal,
                  0.03f,
                  DEBUG_RENDER_RED);
    DebugDrawLine(m_position,
                  m_position + leftNormal,
                  0.03f,
                  DEBUG_RENDER_GREEN);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::UpdateTurret(const float deltaSeconds)
{
    if (m_shootCoolDown > 0.0f)
    {
        m_shootCoolDown -= deltaSeconds;
    }

    // Turn and shoot ( or turn idly)
    PlayerTank* playerTank = g_theGame->GetPlayerTank();
    if (m_map->HasLineOfSight(m_position, playerTank->m_position, SCORPIO_RANGE))
    {
        // Turn toward player
        float const targetOrientationDegrees = (m_playerTankLastKnownPosition - m_position).GetOrientationDegrees();

        TurnToward(m_turretOrientationDegrees, targetOrientationDegrees, deltaSeconds, SCORPIO_ANGULAR_VELOCITY);

        // Shot at player if facing close enough to orientation
        Vec2 const  dispToTarget    = playerTank->m_position - m_position;
        Vec2 const  myFwdNormal     = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
        float const degreesToTarget = GetAngleDegreesBetweenVectors2D(dispToTarget, myFwdNormal);

        if (degreesToTarget < SCORPIO_SHOOT_DEGREES_THRESHOLD &&
            m_shootCoolDown <= 0.0f)
        {
            m_map->SpawnNewEntity(ENTITY_TYPE_BULLET, ENTITY_FACTION_EVIL, m_position + myFwdNormal * 0.45f, m_turretOrientationDegrees);
            m_shootCoolDown = SCORPIO_SHOOT_COOLDOWN;
        }

        m_playerTankLastKnownPosition = playerTank->m_position;
    }
    else
    {
        // turn blindly
        m_turretOrientationDegrees += deltaSeconds * SCORPIO_ANGULAR_VELOCITY;
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

    DebugDrawLine(m_position + fwdNormal * 0.45f, raycastResult2D.m_impactPos, 0.05f, DEBUG_RENDER_RED);
}
