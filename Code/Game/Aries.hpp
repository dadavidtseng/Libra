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
    ~Aries() override;
    
    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;

    AABB2    m_bodyBounds;
    Texture* m_BodyTexture = nullptr;
};
