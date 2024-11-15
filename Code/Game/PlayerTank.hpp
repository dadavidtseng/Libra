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
// protected:
// 	PlayerTank(EntityFaction faction);
// 	virtual ~PlayerTank();

public:
    PlayerTank(Map* map, EntityType type, EntityFaction faction);
    ~PlayerTank() override;

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void UpdateTurret(float deltaSeconds);
    void RenderBody() const;
    void RenderTurret() const;

    float m_turretOrientation            = 0.0f;   // Current orientation of the turret
    float m_turretGoalOrientationDegrees = 0.0f;   // Target orientation for the turret
    float m_turretAngularVelocity        = 360.0f; // Rotation speed for the turret
    Vec2  m_bodyInput                    = Vec2();

    AABB2    m_bodyBounds;
    AABB2    m_turretBounds;
    Texture* m_bodyTexture   = nullptr;
    Texture* m_turretTexture = nullptr;
};
