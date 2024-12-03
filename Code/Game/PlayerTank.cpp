//----------------------------------------------------------------------------------------------------
// PlayerTank.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/PlayerTank.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
PlayerTank::PlayerTank(Map* map, EntityType const type, EntityFaction const faction)
    : Entity(map, type, faction)
{
    m_physicsRadius      = g_gameConfigBlackboard.GetValue("playerTankPhysicsRadius", 0.3f);
    m_rotateSpeed        = g_gameConfigBlackboard.GetValue("playerTankBodyRotateSpeed", 180.f);
    m_health             = g_gameConfigBlackboard.GetValue("playerTankInitHealth", 100);
    m_isPushedByWalls    = g_gameConfigBlackboard.GetValue("playerTankIsPushedByWalls", true);
    m_isPushedByEntities = g_gameConfigBlackboard.GetValue("playerTankIsPushedByEntities", true);
    m_doesPushEntities   = g_gameConfigBlackboard.GetValue("playerTankDoesPushEntities", true);
    m_canSwim            = g_gameConfigBlackboard.GetValue("playerTankCanSwim", false);

    m_bodyBounds   = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));
    m_turretBounds = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));

    m_bodyTexture   = g_theRenderer->CreateOrGetTextureFromFile(PLAYER_TANK_BODY_IMG);
    m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile(PLAYER_TANK_TURRET_IMG);
}

//----------------------------------------------------------------------------------------------------
void PlayerTank::Update(const float deltaSeconds)
{
    if (m_isDead)
        return;

    if (m_health <= 0)
    {
        m_isDead = true;
    }

    UpdateBody(deltaSeconds);
    UpdateTurret(deltaSeconds);

    if (m_shootCoolDown > 0.0f)
    {
        m_shootCoolDown -= deltaSeconds;
    }

    XboxController const& controller = g_theInput->GetController(0);

    if (g_theInput->IsKeyDown(KEYCODE_SPACE) || controller.IsButtonDown(XBOX_BUTTON_A))
    {
        if (m_shootCoolDown <= 0.0f)
        {
            float const turretAbsoluteDegrees = m_orientationDegrees + m_turretRelativeOrientation;
            Vec2 const  fwdNormal             = Vec2::MakeFromPolarDegrees(turretAbsoluteDegrees);
            m_map->SpawnNewEntity(ENTITY_TYPE_BULLET, ENTITY_FACTION_GOOD, m_position + fwdNormal * 0.2f, turretAbsoluteDegrees);
            m_shootCoolDown = g_gameConfigBlackboard.GetValue("playerTankShootCoolDown", 0.1f);

            g_theAudio->StartSound(g_theGame->GetPlayerTankShootSoundID());

            EventArgs args;
            args.SetValue("playerName", "Alice");
            g_theEventSystem->FireEvent("OnBulletSpawned", args);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void PlayerTank::Render() const
{
    if (m_isDead)
        return;

    RenderBody();
    RenderTurret();
}

// #TODO: right now is readable, but should adjust it to match the demo
//----------------------------------------------------------------------------------------------------
void PlayerTank::DebugRender() const
{
    if (m_isDead)
        return;

    const Vec2 fwdNormal  = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
    const Vec2 leftNormal = fwdNormal.GetRotated90Degrees();

    // Outer and inner rings
    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.05f,
                  Rgba8::CYAN); // Inner circle (physics radius)

    // Local space vectors
    DebugDrawLine(m_position,
                  m_position + fwdNormal,
                  0.05f,
                  Rgba8::RED); // i vector (red)
    DebugDrawLine(m_position,
                  m_position + leftNormal,
                  0.05f,
                  Rgba8::GREEN); // j vector (green)

    // Player tank's target and current orientations
    const Vec2 goalOrientationVec    = Vec2::MakeFromPolarDegrees(m_targetOrientationDegrees);
    const Vec2 currentOrientationVec = Vec2::MakeFromPolarDegrees(m_orientationDegrees);

    // Draw target orientation line (short blue line segment outside the circle)
    DebugDrawLine(m_position + goalOrientationVec,
                  m_position + goalOrientationVec * 1.5f,
                  0.15f,
                  Rgba8::BLUE);

    // Draw current orientation line (blue line segment inside the circle)
    DebugDrawLine(m_position,
                  m_position + currentOrientationVec,
                  0.1f,
                  Rgba8::BLUE);

    // TODO: ASK IF THIS IS OKAY!!!
    // Draw turret's current and goal orientations
    const Vec2 turretGoalVec    = Vec2::MakeFromPolarDegrees(m_turretGoalOrientationDegrees);
    const Vec2 turretCurrentVec = Vec2::MakeFromPolarDegrees(m_turretRelativeOrientation + m_orientationDegrees);

    DebugDrawLine(m_position + turretGoalVec,
                  m_position + turretGoalVec * 1.5f,
                  0.075f,
                  Rgba8::GREY);

    DebugDrawLine(m_position,
                  m_position + turretCurrentVec,
                  0.05f,
                  Rgba8::GREY);

    DebugDrawLine(m_position,
                  m_position + m_bodyInput,
                  0.025f,
                  Rgba8::YELLOW);
}

//----------------------------------------------------------------------------------------------------
void PlayerTank::UpdateBody(const float deltaSeconds)
{
    XboxController const& controller = g_theInput->GetController(0);
    m_bodyInput                      = controller.GetLeftStick().GetPosition();

    if (g_theInput->IsKeyDown('W')) m_bodyInput += Vec2(0.f, 1.f);
    if (g_theInput->IsKeyDown('S')) m_bodyInput += Vec2(0.f, -1.f);
    if (g_theInput->IsKeyDown('A')) m_bodyInput += Vec2(-1.f, 0.f);
    if (g_theInput->IsKeyDown('D')) m_bodyInput += Vec2(1.f, 0.f);

    if (m_bodyInput.GetLengthSquared() <= 0.0f)
        return;

    m_bodyInput.ClampLength(1.f);   // if it's over 100%, clamp it

    const Vec2 moveDelta       = m_bodyInput * deltaSeconds;
    m_targetOrientationDegrees = m_bodyInput.GetOrientationDegrees();

    TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, m_rotateSpeed);

    m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * moveDelta.GetLength();
    m_position += m_velocity;

}

//----------------------------------------------------------------------------------------------------
void PlayerTank::UpdateTurret(const float deltaSeconds)
{
    XboxController const& controller  = g_theInput->GetController(0);
    Vec2                  turretInput = controller.GetRightStick().GetPosition();

    if (g_theInput->IsKeyDown('I')) turretInput += Vec2(0.0f, 1.0f);
    if (g_theInput->IsKeyDown('K')) turretInput += Vec2(0.0f, -1.0f);
    if (g_theInput->IsKeyDown('J')) turretInput += Vec2(-1.0f, 0.0f);
    if (g_theInput->IsKeyDown('L')) turretInput += Vec2(1.0f, 0.0f);

    if (turretInput.GetLengthSquared() <= 0.0f)
        return;

    m_turretGoalOrientationDegrees = turretInput.GetOrientationDegrees();

    float turretGoalRelativeOrientation = m_turretGoalOrientationDegrees - m_orientationDegrees;

    TurnToward(m_turretRelativeOrientation, turretGoalRelativeOrientation, deltaSeconds,
               m_turretRotateSpeed);
}

//----------------------------------------------------------------------------------------------------
void PlayerTank::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_bodyBounds, Rgba8::WHITE);

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.0f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_bodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}

//----------------------------------------------------------------------------------------------------
void PlayerTank::RenderTurret() const
{
    std::vector<Vertex_PCU> turretVerts;
    AddVertsForAABB2D(turretVerts, m_turretBounds, Rgba8::WHITE);

    TransformVertexArrayXY3D(static_cast<int>(turretVerts.size()), turretVerts.data(),
                             1.0f, m_orientationDegrees + m_turretRelativeOrientation, m_position);

    g_theRenderer->BindTexture(m_turretTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(turretVerts.size()), turretVerts.data());
}
