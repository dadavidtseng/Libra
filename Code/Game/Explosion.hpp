//----------------------------------------------------------------------------------------------------
// Explosion.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class SpriteSheet;

//----------------------------------------------------------------------------------------------------
class Explosion : public Entity
{
public:
    Explosion(Map* map, EntityType type, EntityFaction faction);
    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void RenderBody() const;

    SpriteSheet* m_spriteSheet   = nullptr;
    float        m_animationTime = 0.f;
};
