//----------------------------------------------------------------------------------------------------
// Scorpio.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class Scorpio : public Entity
{
public:
    Scorpio(Map* map, EntityType type, EntityFaction faction);
    ~Scorpio() override;

    // void TurnTowardPlayer(const Vec2& targetPosition, float maxTurnDegrees, float deltaSeconds);
    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateTurret(float deltaSeconds);
    void RenderBody() const;
    void RenderTurret() const;

    float    m_turretOrientationDegrees = 0.0f;   // Current orientation of the turret
    AABB2    m_bodyBounds;
    AABB2    m_turretBounds;
    Texture* m_bodyTexture   = nullptr;
    Texture* m_turretTexture = nullptr;
    float    m_shootCoolDown = SCORPIO_SHOOT_COOLDOWN;
    Vec2     m_playerTankLastKnownPosition;
};
