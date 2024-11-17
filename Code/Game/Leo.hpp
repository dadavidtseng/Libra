//----------------------------------------------------------------------------------------------------
// Leo.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "GameCommon.hpp"
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

    AABB2    m_BodyBounds;
    Texture* m_BodyTexture                 = nullptr;
    Vec2     m_playerTankLastKnownPosition = Vec2::ZERO;
    float    m_shootCoolDown               = LEO_SHOOT_COOLDOWN;
    bool     m_hasTarget                   = false;
};
