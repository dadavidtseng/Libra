//----------------------------------------------------------------------------------------------------
// Aries.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class Aries : public Entity
{
public:
    Aries(Map* map, EntityType type, EntityFaction faction);

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;
    void WanderAround(float deltaSeconds, float speed);

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;

    AABB2    m_bodyBounds;
    Texture* m_bodyTexture                 = nullptr;
    Vec2     m_playerTankLastKnownPosition = Vec2::ZERO;  // (0,0) means no target
    float    m_timeSinceLastRoll           = 0.f; // Timer to track time since last orientation update
    bool     m_hasTarget                   = false;
};
