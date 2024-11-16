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

    m_bodyBounds    = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_turretBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_bodyTexture   = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_BODY_IMG);
    m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile(SCORPIO_TURRET_IMG);
}

//----------------------------------------------------------------------------------------------------
Scorpio::~Scorpio() = default;

//----------------------------------------------------------------------------------------------------
void Scorpio::Update(const float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    UpdateTurret(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderBody();
    RenderTurret();

    Vec2 const            fwdNormal       = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
    Ray2 const            ray             = Ray2(m_position, fwdNormal.GetNormalized(), 10000);
    RaycastResult2D const raycastResult2D = m_map->RaycastVsTiles(ray);

    DebugDrawLine(m_position, raycastResult2D.m_impactPos, 0.1f, DEBUG_RENDER_RED);
}

//----------------------------------------------------------------------------------------------------
void Scorpio::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;

    Vec2 const fwdNormal  = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
    Vec2 const leftNormal = fwdNormal.GetRotated90Degrees();

    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.05f,
                  DEBUG_RENDER_CYAN);

    DebugDrawLine(m_position,
                  m_position + fwdNormal,
                  0.05f,
                  DEBUG_RENDER_RED);
    DebugDrawLine(m_position,
                  m_position + leftNormal,
                  0.05f,
                  DEBUG_RENDER_GREEN);

    Vec2 const            direction       = (m_playerTankLastKnownPosition - m_position);
    Ray2 const            ray             = Ray2(m_position, direction.GetNormalized(), direction.GetLength());
    RaycastResult2D const raycastResult2D = m_map->RaycastVsTiles(ray);

    if (raycastResult2D.m_didImpact)
    {
        DebugDrawLine(m_position,
                      raycastResult2D.m_impactPos,
                      0.05f,
                      DEBUG_RENDER_RED);
    }
    else
    {
        DebugDrawLine(m_position,
                      m_playerTankLastKnownPosition,
                      0.05f,
                      DEBUG_RENDER_GREY);
    }
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
        const float targetOrientationDegrees = (m_playerTankLastKnownPosition - m_position).GetOrientationDegrees();

        TurnToward(m_turretOrientationDegrees, targetOrientationDegrees, deltaSeconds, SCORPIO_ANGULAR_VELOCITY);

        // Shot at player if facing close enough to orientation
        const Vec2  dispToTarget    = playerTank->m_position - m_position;
        const Vec2  myFwdNormal     = Vec2::MakeFromPolarDegrees(m_turretOrientationDegrees);
        const float degreesToTarget = GetAngleDegreesBetweenVectors2D(dispToTarget, myFwdNormal);

        if (degreesToTarget < SCORPIO_SHOOT_DEGREES_THRESHOLD &&
            m_shootCoolDown <= 0.0f)
        {
            m_map->SpawnNewEntity(ENTITY_BULLET, ENTITY_FACTION_EVIL, m_position, m_turretOrientationDegrees);
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
