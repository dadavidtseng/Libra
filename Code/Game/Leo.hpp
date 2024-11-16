//----------------------------------------------------------------------------------------------------
// Leo.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class Leo : public Entity
{
public:
    Leo(Map* map, EntityType type, EntityFaction faction);

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;
    void UpdateShootCoolDown(float deltaSeconds);
    void WanderAround(float deltaSeconds, float speed);

    AABB2    m_BodyBounds;
    Texture* m_BodyTexture                 = nullptr;
    Vec2     m_playerTankLastKnownPosition = Vec2::ZERO;  // (0,0) means no target
    float    m_timeSinceLastRoll           = 0.f; // Timer to track time since last orientation update
    float    m_shootCoolDown               = 0.f;
    bool     m_hasTarget                   = false;
};
