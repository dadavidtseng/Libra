//----------------------------------------------------------------------------------------------------
// TileDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/TileDefinition.hpp"

//----------------------------------------------------------------------------------------------------
class SpriteSheet;

//----------------------------------------------------------------------------------------------------
std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

//----------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition(XmlElement const& tileDefElement, SpriteSheet const& spriteSheet)
{
    m_name                = ParseXmlAttribute(tileDefElement, "name", "Unnamed");
    int const spriteIndex = ParseXmlAttribute(tileDefElement, "spriteIndex", -1);

    if (spriteIndex != -1)
    {
        m_spriteDef = spriteSheet.GetSpriteDef(spriteIndex);
    }

    m_isSolid   = ParseXmlAttribute(tileDefElement, "isSolid", false);
    m_tintColor = ParseXmlAttribute(tileDefElement, "tintColor", Rgba8::WHITE);
}

//----------------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
    for (TileDefinition const* tileDef : s_tileDefinitions)
    {
        delete tileDef;
    }

    s_tileDefinitions.clear();
}

//----------------------------------------------------------------------------------------------------
void TileDefinition::InitializeTileDefs(SpriteSheet const& spriteSheet)
{
    XmlDocument tileDefXml;
    if (tileDefXml.LoadFile("Data/Definitions/TileDefinitions.xml") != XmlResult::XML_SUCCESS)
    {
        return;
    }

    if (XmlElement* root = tileDefXml.FirstChildElement("TileDefinitions"))
    {
        for (XmlElement* element = root->FirstChildElement("TileDefinition"); element != nullptr; element = element->NextSiblingElement("TileDefinition"))
        {
            TileDefinition* tileDef = new TileDefinition(*element, spriteSheet);
            s_tileDefinitions.push_back(tileDef);
        }
    }
}

//----------------------------------------------------------------------------------------------------
TileDefinition const* TileDefinition::GetTileDefByName(String const& name)
{
    for (TileDefinition const* tileDef : s_tileDefinitions)
    {
        if (tileDef->GetName() == name)
        {
            return tileDef;
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
Strings TileDefinition::GetTileNames()
{
    Strings tileNames;

    for (TileDefinition const* tileDef : s_tileDefinitions)
    {
        if (tileDef)
        {
            tileNames.push_back(tileDef->GetName());
        }
    }

    return tileNames;
}
