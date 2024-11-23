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

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateTurret(float deltaSeconds);
    void RenderBody() const;
    void RenderTurret() const;
    void RenderLaser() const;

    AABB2    m_bodyBounds;
    AABB2    m_turretBounds;
    Texture* m_bodyTexture                 = nullptr;
    Texture* m_turretTexture               = nullptr;
    float    m_turretOrientationDegrees    = 0.f;
    float    m_shootCoolDown               = SCORPIO_SHOOT_COOLDOWN;
    Vec2     m_playerTankLastKnownPosition = Vec2::ZERO;
};
