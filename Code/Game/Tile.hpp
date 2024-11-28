//----------------------------------------------------------------------------------------------------
// Tile.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

//----------------------------------------------------------------------------------------------------
// "Flyweight" design pattern ( each tile only knows its type )
struct Tile
{
    // delete this for P.O.D
    // Tile(const IntVec2& coords, TileType type);
    // Tile();
    // ~Tile();
    // void SetType(std::string const& tileDefName);

    IntVec2 m_tileCoords = IntVec2(-1, -1);
    String  m_tileName;
    bool    m_isSolid = false;
    // int     m_tileDefIndex = -1;
};
