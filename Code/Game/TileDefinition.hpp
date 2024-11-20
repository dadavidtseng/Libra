//----------------------------------------------------------------------------------------------------
// TileDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//----------------------------------------------------------------------------------------------------
enum TileType: int
{
    // TODO: remove for future assignment
    TILE_TYPE_INVALID = -1,
    TILE_TYPE_GRASS,
    TILE_TYPE_STONE,
    TILE_TYPE_SPARKLE_01,
    TILE_TYPE_SPARKLE_02,
    TILE_TYPE_FLOOR,
    TILE_TYPE_EXIT,
    NUM_TILE_TYPE
};

//----------------------------------------------------------------------------------------------------
// class TileDefinition
struct TileDefinition
{
public:
//TileDefinition (XmlElement const& tileDefElement);
    TileDefinition() = default;
    TileDefinition(TileType tileType, SpriteDefinition const& spriteDef, bool isSolid = false, Rgba8 const& tintColor = Rgba8::WHITE);

    static void           InitializeTileDefinitions(SpriteSheet const& spriteSheet);
    static TileDefinition const& GetTileDefinition(TileType tileType);  // pass by pointer

    bool                               IsSolid() const { return m_isSolid; }
    Rgba8                              GetTintColor() const { return m_tintColor; }
    AABB2                              GetUVs() const { return m_spriteDef.GetUVs(); }
    TileType                           GetTileType() const { return m_tileType; }
    SpriteDefinition                   GetSpriteDefinition() const { return m_spriteDef; }
    static std::vector<TileDefinition> s_tileDefinitions;   // store by pointer   

private:
    TileType         m_tileType;
    SpriteDefinition m_spriteDef;
    bool             m_isSolid;
    Rgba8            m_tintColor;
};
