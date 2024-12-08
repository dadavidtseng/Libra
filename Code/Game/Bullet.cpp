//----------------------------------------------------------------------------------------------------
// Bullet.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Bullet.hpp"

#include "PlayerTank.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------
Bullet::Bullet(Map* map, EntityType const type, EntityFaction const faction)
    : Entity(map, type, faction)
{
    m_isPushedByWalls    = g_gameConfigBlackboard.GetValue("bulletIsPushedByWalls", true);
    m_isPushedByEntities = g_gameConfigBlackboard.GetValue("bulletIsPushedByEntities", true);
    m_doesPushEntities   = g_gameConfigBlackboard.GetValue("bulletDoesPushEntities", true);

    if (faction == ENTITY_FACTION_GOOD)
    {
        m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(BULLET_GOOD_IMG);
        m_health      = g_gameConfigBlackboard.GetValue("bulletGoodInitHealth", 3);
        m_moveSpeed   = g_gameConfigBlackboard.GetValue("bulletGoodMoveSpeed", 5.f);
    }
    if (faction == ENTITY_FACTION_EVIL)
    {
        m_BodyTexture = g_theRenderer->CreateOrGetTextureFromFile(BULLET_EVIL_IMG);
        m_health      = g_gameConfigBlackboard.GetValue("bulletEvilInitHealth", 1);
        m_moveSpeed   = g_gameConfigBlackboard.GetValue("bulletEvilMoveSpeed", 3.f);
    }

    m_BodyBounds    = AABB2(Vec2(-0.1f, -0.05f), Vec2(0.1f, 0.05f));
    m_physicsRadius = GetDistance2D(m_BodyBounds.m_mins, m_BodyBounds.m_maxs) * 0.5f;
}

//----------------------------------------------------------------------------------------------------
void Bullet::Update(float const deltaSeconds)
{
    if (m_isDead)
        return;

    UpdateBody(deltaSeconds);

    if (m_health <= 0)
    {
        m_isGarbage = true;
        m_isDead    = true;
    }
}

//----------------------------------------------------------------------------------------------------
void Bullet::Render() const
{
    if (m_isDead)
        return;

    RenderBody();
}

//----------------------------------------------------------------------------------------------------
void Bullet::DebugRender() const
{
    if (m_isDead)
        return;

    DebugDrawRing(m_position,
                  m_physicsRadius,
                  0.03f,
                  Rgba8::CYAN);
}

//----------------------------------------------------------------------------------------------------
void Bullet::UpdateBody(float const deltaSeconds)
{
    m_velocity = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_moveSpeed);

    Ray2 ray = Ray2(m_position, m_velocity,0.05f);
RaycastResult2D raycastResult2D = m_map->RaycastVsTiles(ray);
    
    Vec2 const nextPosition = m_position + m_velocity * deltaSeconds;

    if (raycastResult2D.m_didImpact)
    {
        m_health--;

        // IntVec2 const normalOfSurfaceToReflectOffOf = m_map->GetTileCoordsFromWorldPos(m_position) - m_map->GetTileCoordsFromWorldPos(nextPosition);
        IntVec2 const normalOfSurfaceToReflectOffOf = IntVec2(raycastResult2D.m_impactNormal);
        printf("(%f, %f)\n", raycastResult2D.m_impactNormal.x, raycastResult2D.m_impactNormal.y);
        Vec2 const    ofSurfaceToReflectOffOf(static_cast<float>(normalOfSurfaceToReflectOffOf.x), static_cast<float>(normalOfSurfaceToReflectOffOf.y));
        Vec2 const    reflectedVelocity = m_velocity.GetReflected(ofSurfaceToReflectOffOf.GetNormalized());
        m_orientationDegrees            = Atan2Degrees(reflectedVelocity.y, reflectedVelocity.x);
    }
    else
    {
        m_position = nextPosition;
    }
}

//----------------------------------------------------------------------------------------------------
void Bullet::RenderBody() const
{
    VertexList bodyVerts;
    AddVertsForAABB2D(bodyVerts, m_BodyBounds, Rgba8(255, 255, 255));

    TransformVertexArrayXY3D(static_cast<int>(bodyVerts.size()), bodyVerts.data(),
                             1.f, m_orientationDegrees, m_position);

    g_theRenderer->BindTexture(m_BodyTexture);
    g_theRenderer->DrawVertexArray(static_cast<int>(bodyVerts.size()), bodyVerts.data());
}
