//----------------------------------------------------------------------------------------------------
// Explosion.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class Explosion : public Entity
{
public:
    Explosion(Map* map, EntityType type, EntityFaction faction);
    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;

    AABB2    m_BodyBounds;
    Texture* m_BodyTexture = nullptr;
};
