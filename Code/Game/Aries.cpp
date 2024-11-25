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
    m_physicsRadius               = g_gameConfigBlackboard.GetValue("ariesPhysicsRadius", 0.25f);
    m_detectRange                 = g_gameConfigBlackboard.GetValue("ariesDetectRange", 10.f);
    m_moveSpeed                   = g_gameConfigBlackboard.GetValue("ariesMoveSpeed", 0.5f);
    m_rotateSpeed                 = g_gameConfigBlackboard.GetValue("ariesRotateSpeed", 90.f);
    m_health                      = g_gameConfigBlackboard.GetValue("ariesInitHealth", 8);
    m_isPushedByWalls             = g_gameConfigBlackboard.GetValue("ariesIsPushedByWalls", true);
    m_isPushedByEntities          = g_gameConfigBlackboard.GetValue("ariesIsPushedByEntities", true);
    m_doesPushEntities            = g_gameConfigBlackboard.GetValue("ariesDoesPushEntities", true);
    m_playerTankLastKnownPosition = m_position;
    
    m_bodyBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_bodyTexture = g_theRenderer->CreateOrGetTextureFromFile(ARIES_BODY_IMG);
}

//----------------------------------------------------------------------------------------------------
void Aries::Update(const float deltaSeconds)
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

    UpdateBody(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Aries::Render() const
{
    if (m_isDead)
        return;

    RenderBody();
}

//----------------------------------------------------------------------------------------------------
void Aries::DebugRender() const
{
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

    if (IsPointInsideDisc2D(m_playerTankLastKnownPosition, m_position, m_physicsRadius) ||
        playerTank->m_isDead)
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

        TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, m_rotateSpeed);

        m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * m_moveSpeed * deltaSeconds;
        m_position += m_velocity;

    }

    if (m_map->HasLineOfSight(m_position, playerTank->m_position, m_detectRange))
    {
        m_hasTarget = true;

        m_playerTankLastKnownPosition = playerTank->m_position;

        float const targetOrientationDegrees = (playerTank->m_position - m_position).GetOrientationDegrees();

        TurnToward(
            m_orientationDegrees,
            targetOrientationDegrees,
            deltaSeconds,
            m_rotateSpeed);
    }
    else
    {
        WanderAround(deltaSeconds, m_moveSpeed, m_rotateSpeed);
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
