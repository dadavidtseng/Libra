//----------------------------------------------------------------------------------------------------
// Entity.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Math/Vec2.hpp"

//----------------------------------------------------------------------------------------------------
class Map;

//----------------------------------------------------------------------------------------------------
enum EntityType: int
{
    ENTITY_TYPE_UNKNOWN = -1,
    ENTITY_TYPE_PLAYER_TANK,
    ENTITY_SCORPIO,
    ENTITY_LEO,
    ENTITY_ARIES,
    ENTITY_BULLET,
    NUM_ENTITY_TYPES
};

//----------------------------------------------------------------------------------------------------
enum EntityFaction: int
{
    ENTITY_FACTION_UNKNOWN = -1,
    ENTITY_FACTION_GOOD,
    ENTITY_FACTION_NEUTRAL,
    ENTITY_FACTION_EVIL,
    NUM_ENTITY_FACTIONS
};

//-----------------------------------------------------------------------------------------------
class Entity
{
    friend class Map;

public:
    Entity(Map* map, EntityType  type, EntityFaction faction);
    virtual ~Entity() = default; //add an addition secrete pointer to the class

    virtual void Update(float deltaSeconds) = 0; // Pure virtual function for updating the entity
    virtual void Render() const = 0;             // Pure virtual function for rendering the entity
    virtual void DebugRender() const = 0;        // Pure virtual function for debug rendering the entity
    virtual void TurnToward(float& orientationDegrees, float targetOrientationDegrees, float deltaSeconds, float rotationSpeed);
    void         MoveToward(Vec2& currentPosition, Vec2 const& targetPosition, float speed, float deltaSeconds);

// TODO: MAKE THIS
// virtual  void TurnTowardPosition(Vec2 const& targetPos, float maxTurnDegrees); 

    Map*          m_map = nullptr;
    EntityType    m_type;
    EntityFaction m_faction;
    Vec2          m_position                 = Vec2();           // 2D Cartesian origin in world space
    float         m_orientationDegrees       = 0.f; // Forward angle, counter-clockwise from +x/east
    Vec2          m_velocity                 = Vec2(0.f, 0.f);           // Linear velocity in world units per second
    float         m_targetOrientationDegrees = 0;
    float         m_physicsRadius            = 0;   // Conservative collision radius
    int           m_health                   = 0;
};
