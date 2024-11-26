//----------------------------------------------------------------------------------------------------
// MapDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/MapDefinition.hpp"

//----------------------------------------------------------------------------------------------------
std::vector<MapDefinition*> MapDefinition::s_mapDefinitions;

//----------------------------------------------------------------------------------------------------
MapDefinition::MapDefinition(XmlElement const& mapDefElement)
{
    m_index                  = ParseXmlAttribute(mapDefElement, "index", -1);
    m_scorpioSpawnPercentage = ParseXmlAttribute(mapDefElement, "scorpioSpawnPercentage", -1.f);
    m_leoSpawnPercentage     = ParseXmlAttribute(mapDefElement, "leoSpawnPercentage", -1.f);
    m_ariesSpawnPercentage   = ParseXmlAttribute(mapDefElement, "ariesSpawnPercentage", -1.f);
    m_dimensions             = ParseXmlAttribute(mapDefElement, "dimensions", IntVec2(-1, -1));
}

//----------------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
    for (MapDefinition const* mapDef : s_mapDefinitions)
    {
        delete mapDef;
    }

    s_mapDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
void MapDefinition::InitializeMapDefs()
{
    XmlDocument mapDefXml;
    if (mapDefXml.LoadFile("Data/Definitions/MapDefinitions.xml") != XmlResult::XML_SUCCESS)
    {
        return;
    }

    if (XmlElement* root = mapDefXml.FirstChildElement("MapDefinitions"))
    {
        for (XmlElement* element = root->FirstChildElement("MapDefinition"); element != nullptr; element = element->NextSiblingElement("MapDefinition"))
        {
            MapDefinition* tileDef = new MapDefinition(*element);
            s_mapDefinitions.push_back(tileDef);
        }
    }
}

// //----------------------------------------------------------------------------------------------------
// MapDefinition const* MapDefinition::GetTileDefByName(String const& name)
// {
//     for (MapDefinition const* tileDef : s_mapDefinitions)
//     {
//         if (tileDef->GetName() == name)
//         {
//             return tileDef;
//         }
//     }
//
//     return nullptr;
// }
