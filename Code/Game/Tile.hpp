//----------------------------------------------------------------------------------------------------
// Tile.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <string>

#include "Engine/Math/IntVec2.hpp"
#include "Game/TileDefinition.hpp"

//----------------------------------------------------------------------------------------------------
enum TileType;

//----------------------------------------------------------------------------------------------------
// "Flyweight" design pattern ( each tile only knows its type )
struct Tile
{
    // delete this for P.O.D
    // Tile(const IntVec2& coords, TileType type);
    // Tile();
    // ~Tile();
    // void SetType(std::string const& tileDefName);
    

    IntVec2  m_tileCoords = IntVec2(-1, -1);
    TileType m_type       = TILE_TYPE_INVALID;
    // int m_tileDefIndex = -1;
};
