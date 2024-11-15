//----------------------------------------------------------------------------------------------------
// TileDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/TileDefinition.hpp"

//----------------------------------------------------------------------------------------------------
class SpriteSheet;

//----------------------------------------------------------------------------------------------------
std::vector<TileDefinition> TileDefinition::s_tileDefinitions;

//----------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition(TileType const tileType, SpriteDefinition const& spriteDef, bool const isSolid, Rgba8 const& tintColor)
    : m_tileType(tileType),
      m_spriteDef(spriteDef),
      m_isSolid(isSolid),
      m_tintColor(tintColor)
{
}

//----------------------------------------------------------------------------------------------------
void TileDefinition::InitializeTileDefinitions(SpriteSheet const& spriteSheet)
{
    s_tileDefinitions.reserve(NUM_TILE_TYPE);

    // Define each tile type and its properties
    s_tileDefinitions.emplace_back(TILE_TYPE_GRASS, spriteSheet.GetSpriteDef(0), false, Rgba8::WHITE);
    s_tileDefinitions.emplace_back(TILE_TYPE_STONE, spriteSheet.GetSpriteDef(17), true, Rgba8::WHITE);

    // Add other tile types as necessary
}

//----------------------------------------------------------------------------------------------------
TileDefinition const& TileDefinition::GetTileDefinition(TileType const tileType)
{
    return s_tileDefinitions[tileType];
}
