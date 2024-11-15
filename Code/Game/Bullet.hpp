//----------------------------------------------------------------------------------------------------
// Bullet.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------
class Bullet : public Entity
{
public:
    Bullet(Map* map, EntityType type, EntityFaction faction);
    ~Bullet() override;
    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;

    AABB2    m_BodyBounds;
    Texture* m_BodyTexture = nullptr;
};
