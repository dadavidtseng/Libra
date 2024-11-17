//----------------------------------------------------------------------------------------------------
// Bullet.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Bullet.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
Bullet::Bullet(Map* map, const EntityType type, const EntityFaction faction)
    : Entity(map, type, faction)
{
    m_isPushedByWalls    = false;
    m_isPushedByEntities = false;
    m_doesPushEntities   = false;
    
    if (faction == ENTITY_FACTION_GOOD)
    {
        m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(BULLET_GOOD_IMG);
    }
    if (faction == ENTITY_FACTION_EVIL)
    {
        m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(BULLET_EVIL_IMG);
    }

    m_BodyBounds    = AABB2(Vec2(-0.1f, -0.05f), Vec2(0.1f, 0.05f));
    m_physicsRadius = GetDistance2D(m_BodyBounds.m_mins, m_BodyBounds.m_maxs) * 0.5f;
}

//----------------------------------------------------------------------------------------------------
Bullet::~Bullet() = default;

//----------------------------------------------------------------------------------------------------
void Bullet::Update(const float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    UpdateBody(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Bullet::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderBody();
}

//----------------------------------------------------------------------------------------------------
void Bullet::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;
    
    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.03f,
                  DEBUG_RENDER_CYAN);
}

//----------------------------------------------------------------------------------------------------
void Bullet::UpdateBody(float deltaSeconds)
{
    m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, BULLET_EVIL_SPEED);

    Vec2 nextPosition = m_position + m_velocity * deltaSeconds;

    if (m_map->IsTileSolid(m_map->GetTileCoordsFromWorldPos(nextPosition)))
    {
        IntVec2 normalOfSurfaceToReflectOffOf = m_map->GetTileCoordsFromWorldPos(m_position) - m_map->GetTileCoordsFromWorldPos(nextPosition);
        Vec2    ofSurfaceToReflectOffOf(static_cast<float>(normalOfSurfaceToReflectOffOf.x), static_cast<float>(normalOfSurfaceToReflectOffOf.y));
        Vec2    reflectedVelocity = m_velocity.GetReflected(ofSurfaceToReflectOffOf.GetNormalized());
        m_orientationDegrees      = Atan2Degrees(reflectedVelocity.y, reflectedVelocity.x);
    }
    else
    {
        m_position = nextPosition;
    }
}

//----------------------------------------------------------------------------------------------------
void Bullet::RenderBody() const
{
    std::vector<Vertex_PCU> bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_BodyBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_BodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}
