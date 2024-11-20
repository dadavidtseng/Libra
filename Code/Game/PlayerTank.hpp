//----------------------------------------------------------------------------------------------------
// PlayerTank.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class PlayerTank : public Entity
{
public:
    PlayerTank(Map* map, EntityType type, EntityFaction faction);

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void UpdateTurret(float deltaSeconds);
    void RenderBody() const;
    void RenderTurret() const;

    AABB2    m_bodyBounds;
    AABB2    m_turretBounds;
    Texture* m_bodyTexture                  = nullptr;
    Texture* m_turretTexture                = nullptr;
    float    m_turretRelativeOrientation            = 0.0f;
    float    m_turretGoalOrientationDegrees = 0.0f;
    float    m_turretMaxRotateSpeed        = 360.0f;
    float    m_shootCoolDown                = 0.f;
    Vec2     m_bodyInput                    = Vec2();
};
