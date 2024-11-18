//----------------------------------------------------------------------------------------------------
// Aries.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Aries.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"

//----------------------------------------------------------------------------------------------------
Aries::Aries(Map* map, EntityType const type, EntityFaction const faction)
    : Entity(map, type, faction)
{
    m_physicsRadius               = ARIES_PHYSICS_RADIUS;
    m_playerTankLastKnownPosition = m_position;

    m_isPushedByWalls    = true;
    m_isPushedByEntities = true;
    m_doesPushEntities   = true;

    m_health = 3;

    m_bodyBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile(ARIES_BODY_IMG);
}

//----------------------------------------------------------------------------------------------------
void Aries::Update(const float deltaSeconds)
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

    UpdateBody(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Aries::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (m_isDead)
        return;

    RenderBody();
}

//----------------------------------------------------------------------------------------------------
void Aries::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;

    if (m_isDead)
        return;

    Vec2 const fwdNormal  = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
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

    if (m_playerTankLastKnownPosition != Vec2::ZERO)
    {
        DebugDrawLine(m_position,
                      m_playerTankLastKnownPosition,
                      0.05f,
                      DEBUG_RENDER_GREY);

        DebugDrawGlowCircle(m_playerTankLastKnownPosition, 0.1f, DEBUG_RENDER_GREY, 1.f);
    }

    DebugDrawLine(m_position,
                  m_position + m_velocity,
                  0.025f,
                  DEBUG_RENDER_YELLOW);
}

//----------------------------------------------------------------------------------------------------
void Aries::UpdateBody(const float deltaSeconds)
{
    m_timeSinceLastRoll += deltaSeconds;

    const PlayerTank* playerTank = g_theGame->GetPlayerTank();

    if (playerTank->m_isDead)
        m_hasTarget = false;
    
    if (IsPointInsideDisc2D(m_playerTankLastKnownPosition, m_position, m_physicsRadius))
    {
        m_playerTankLastKnownPosition = Vec2::ZERO;
        m_hasTarget                   = false;
    }

    Vec2 const  dispToTarget    = m_playerTankLastKnownPosition - m_position;
    Vec2 const  fwdNormal       = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
    float const degreesToTarget = GetAngleDegreesBetweenVectors2D(dispToTarget, fwdNormal);

    if (degreesToTarget < 45.f && m_hasTarget)
    {
        m_targetOrientationDegrees = Atan2Degrees(dispToTarget.y, dispToTarget.x);

        TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, ARIES_ANGULAR_VELOCITY);

        m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * ARIES_MOVE_SPEED * deltaSeconds;
        m_position += m_velocity;

    }

    if (m_map->HasLineOfSight(m_position, playerTank->m_position, ARIES_RANGE))
    {
        m_hasTarget = true;

        m_playerTankLastKnownPosition = playerTank->m_position;

        float const targetOrientationDegrees = (playerTank->m_position - m_position).GetOrientationDegrees();

        TurnToward(
            m_orientationDegrees,
            targetOrientationDegrees,
            deltaSeconds,
            ARIES_ANGULAR_VELOCITY);
    }
    else
    {
        WanderAround(deltaSeconds, ARIES_MOVE_SPEED, ARIES_ANGULAR_VELOCITY);
    }
}

//----------------------------------------------------------------------------------------------------
void Aries::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_bodyBounds, Rgba8::WHITE);

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.0f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_bodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}
