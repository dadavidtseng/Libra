//----------------------------------------------------------------------------------------------------
// Aries.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Aries.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
Aries::Aries(Map* map, const EntityType type, const EntityFaction faction)
    : Entity(map, type, faction)
{
    m_physicsRadius   = PLAYER_TANK_PHYSICS_RADIUS;
    // m_targetOrientationDegrees =

    m_bodyBounds = AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f));

    m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(ENEMY_TANK_ARIES_BODY_IMG);
}

//----------------------------------------------------------------------------------------------------
Aries::~Aries() = default;

//----------------------------------------------------------------------------------------------------
void Aries::Update(const float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    UpdateBody(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Aries::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderBody();
}

// #TODO: right now is readable, but should adjust it to match the demo
//----------------------------------------------------------------------------------------------------
void Aries::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;

    const Vec2 fwdNormal  = Vec2::MakeFromPolarDegrees(m_orientationDegrees);
    const Vec2 leftNormal = fwdNormal.GetRotated90Degrees();

    // Outer and inner rings
    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.05f,
                  DEBUG_RENDER_CYAN); // Inner circle (physics radius)
    

    // Local space vectors
    DebugDrawLine(m_position,
                  m_position + fwdNormal,
                  0.05f,
                  DEBUG_RENDER_RED); // i vector (red)
    DebugDrawLine(m_position,
                  m_position + leftNormal,
                  0.05f,
                  DEBUG_RENDER_GREEN); // j vector (green)

    // Player tank's target and current orientations
    const Vec2 goalOrientationVec    = Vec2::MakeFromPolarDegrees(m_targetOrientationDegrees);
    const Vec2 currentOrientationVec = Vec2::MakeFromPolarDegrees(m_orientationDegrees);

    // Draw target orientation line (short blue line segment outside the circle)
    

    // Draw current orientation line (blue line segment inside the circle)
    DebugDrawLine(m_position, m_position + currentOrientationVec, 0.25f, DEBUG_RENDER_BLUE);

    
    

    // Velocity vector
    DebugDrawLine(m_position,
                  m_position + m_velocity,
                  0.025f,
                  DEBUG_RENDER_YELLOW);
}

//----------------------------------------------------------------------------------------------------
void Aries::UpdateBody(const float deltaSeconds)
{
    // XboxController const& controller = g_theInput->GetController(0);
    // m_bodyInput                      = controller.GetLeftStick().GetPosition();
    //
    // if (g_theInput->IsKeyDown('W')) m_bodyInput += Vec2(0.f, 1.f);
    // if (g_theInput->IsKeyDown('S')) m_bodyInput += Vec2(0.f, -1.f);
    // if (g_theInput->IsKeyDown('A')) m_bodyInput += Vec2(-1.f, 0.f);
    // if (g_theInput->IsKeyDown('D')) m_bodyInput += Vec2(1.f, 0.f);
    //
    // if (m_bodyInput.GetLengthSquared() > 0.0f)
    // {
    //     const Vec2 moveDelta       = m_bodyInput * deltaSeconds;
    //     m_targetOrientationDegrees = Atan2Degrees(m_bodyInput.y, m_bodyInput.x);
    //
    //     TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, m_angularVelocity);
    //
    //     m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees) * moveDelta.GetLength();
    //     SetPosition(m_velocity);
    // }
}

//----------------------------------------------------------------------------------------------------
void Aries::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_bodyBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.0f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_BodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}
