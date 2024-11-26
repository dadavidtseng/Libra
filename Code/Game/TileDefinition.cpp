//----------------------------------------------------------------------------------------------------
// TileDefinition.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/TileDefinition.hpp"

//----------------------------------------------------------------------------------------------------
class SpriteSheet;

//----------------------------------------------------------------------------------------------------
std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

TileDefinition::TileDefinition(XmlElement const& tileDefElement, SpriteSheet const& spriteSheet)
{
    // Extract the name attribute
    char const* name = tileDefElement.Attribute("name");
    if (name != nullptr)
    {
        m_name = String(name);
    }

    // Extract the spriteIndex attribute
    int const spriteIndex = tileDefElement.IntAttribute("spriteIndex", -1);

    if (spriteIndex != -1)
    {
        // Assuming you have a way to get the sprite definition from the sprite sheet
        m_spriteDef = spriteSheet.GetSpriteDef(spriteIndex);
    }

    // Extract the isSolid attribute
    m_isSolid = tileDefElement.BoolAttribute("isSolid", false);

    // Extract the tint attribute
    char const* tintStr = tileDefElement.Attribute("tintColor");
    
    if (tintStr != nullptr)
    {
        m_tintColor.SetFromText(tintStr);
    }
    else
    {
        m_tintColor = Rgba8::WHITE; // Default tint color
    }
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
        // Handle error (e.g., log it, throw an exception, etc.)
        return;
    }

    if (XmlElement* root = tileDefXml.FirstChildElement("TileDefinitions"))
    {
        for (XmlElement* element = root->FirstChildElement("TileDefinition"); element != nullptr; element = element->NextSiblingElement("TileDefinition"))
        {
            // Create a TileDefinition from the XML element
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
