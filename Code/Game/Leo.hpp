//----------------------------------------------------------------------------------------------------
// Leo.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------
class TileHeatMap;
class Texture;

//----------------------------------------------------------------------------------------------------
class Leo : public Entity
{
public:
    Leo(Map* map, EntityType type, EntityFaction faction);
    void DebugRenderTileIndex() const;

    void Update(float deltaSeconds) override;
    void Render() const override;
    void DebugRender() const override;

private:
    void UpdateBody(float deltaSeconds);
    void RenderBody() const;
    void UpdateShootCoolDown(float deltaSeconds);

    AABB2        m_BodyBounds;
    Texture*     m_BodyTexture                 = nullptr;
    float        m_shootCoolDown               = 0.f;
    float        m_shootDegreesThreshold       = g_gameConfigBlackboard.GetValue("leoShootDegreesThreshold", 5.f);
};
