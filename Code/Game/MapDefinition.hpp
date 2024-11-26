//----------------------------------------------------------------------------------------------------
// MapDefinition.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//----------------------------------------------------------------------------------------------------
struct MapDefinition
{
    explicit MapDefinition(XmlElement const& mapDefElement);
    ~MapDefinition();

    static void                        InitializeMapDefs();
    // static MapDefinition const*        GetTileDefByName(String const& name);
    static std::vector<MapDefinition*> s_mapDefinitions;

    int     GetIndex() const { return m_index; }
    float   GetScorpioSpawnPercentage() const { return m_scorpioSpawnPercentage; }
    float   GetLeoSpawnPercentage() const { return m_leoSpawnPercentage; }
    float   GetAriesSpawnPercentage() const { return m_ariesSpawnPercentage; }
    IntVec2 GetDimensions() const { return m_dimensions; }

private:
    int     m_index;
    float   m_scorpioSpawnPercentage;
    float   m_leoSpawnPercentage;
    float   m_ariesSpawnPercentage;
    IntVec2 m_dimensions;
};
