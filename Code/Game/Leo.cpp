//----------------------------------------------------------------------------------------------------
// Leo.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Leo.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"

//----------------------------------------------------------------------------------------------------
Leo::Leo(Map* map, EntityType const type, EntityFaction const faction)
    : Entity(map, type, faction)
{
    m_physicsRadius   = LEO_PHYSICS_RADIUS;

    m_BodyBounds  = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(ENEMY_TANK_LEO_BODY_IMG);
}

//----------------------------------------------------------------------------------------------------
void Leo::Update(const float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    m_timeSinceLastRoll += deltaSeconds;

    UpdateBody(deltaSeconds);
    if (m_shootCoolDown > 0.0f)
    {
        m_shootCoolDown -= deltaSeconds;
    }
    //Update target position to player position if visible
    PlayerTank* playerTank = g_theGame->GetPlayerTank();

    //Check if target is reached, but not seen; go back wander
    if (IsPointInsideDisc2D(m_playerTankLastKnownPosition, m_position, m_physicsRadius))
    {
        // Clear my target; the player is nowhere to be seen from last known position
        m_playerTankLastKnownPosition = Vec2::ZERO;
        m_hasTarget                   = false;
    }

    //Drive forward if aligned to target position
    Vec2 const  dispToTarget    = m_playerTankLastKnownPosition - m_position;
    Vec2 const  fwdNormal       = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
    float const degreesToTarget = GetAngleDegreesBetweenVectors2D(dispToTarget, fwdNormal);

    if (degreesToTarget < 45.f && m_hasTarget)
    {
        m_targetOrientationDegrees = Atan2Degrees(dispToTarget.y, dispToTarget.x);

        TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, LEO_ANGULAR_VELOCITY);

        m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * LEO_SPEED * deltaSeconds;
        SetPosition(m_position + m_velocity);

        if (degreesToTarget < 5.f &&
            m_shootCoolDown <= 0.0f)
        {
            m_map->SpawnNewEntity(ENTITY_BULLET, ENTITY_FACTION_EVIL, m_position, m_orientationDegrees);
            m_shootCoolDown = SCORPIO_SHOOT_COOLDOWN;
        }
    }

    //Turn(and maybe drive) toward target position
    if (m_map->HasLineOfSight(m_position, playerTank->m_position, LEO_RANGE))
    {
        m_hasTarget = true;

        m_playerTankLastKnownPosition = playerTank->m_position;

        float const targetOrientationDegrees = (playerTank->GetPosition() - m_position).GetOrientationDegrees();

        TurnToward(
            m_orientationDegrees,
            targetOrientationDegrees,
            deltaSeconds,
            LEO_ANGULAR_VELOCITY);
    }
    else
    {
        if (m_timeSinceLastRoll >= 2.0f)
        {
            m_targetOrientationDegrees = static_cast<float>(g_theRNG->RollRandomIntInRange(0, 360));
            m_timeSinceLastRoll        = 0.f;
        }

        TurnToward(m_orientationDegrees,
                   m_targetOrientationDegrees,
                   deltaSeconds,
                   LEO_ANGULAR_VELOCITY);

        m_velocity = fwdNormal * LEO_SPEED * deltaSeconds;
        SetPosition(m_position + m_velocity);
    }
}

//----------------------------------------------------------------------------------------------------
void Leo::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderBody();
}

//----------------------------------------------------------------------------------------------------
void Leo::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
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
                  m_position + fwdNormal,
                  0.025f,
                  DEBUG_RENDER_YELLOW);
}

//----------------------------------------------------------------------------------------------------
void Leo::UpdateBody(const float deltaSeconds)
{

}

//----------------------------------------------------------------------------------------------------
void Leo::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_BodyBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.0f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_BodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}
