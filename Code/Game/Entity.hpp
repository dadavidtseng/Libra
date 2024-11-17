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
    ENTITY_TYPE_SCORPIO,
    ENTITY_TYPE_LEO,
    ENTITY_TYPE_ARIES,
    ENTITY_TYPE_BULLET,
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
    Entity(Map* map, EntityType type, EntityFaction faction);
    virtual ~Entity() = default; //add an addition secrete pointer to the class

    virtual void Update(float deltaSeconds) = 0;
    virtual void Render() const = 0;             
    virtual void DebugRender() const = 0;        
    virtual void TurnToward(float& orientationDegrees, float targetOrientationDegrees, float deltaSeconds, float rotationSpeed);
    void         MoveToward(Vec2& currentPosition, Vec2 const& targetPosition, float moveSpeed, float deltaSeconds);
    void         WanderAround(float deltaSeconds, float moveSpeed, float rotateSpeed);

// TODO: MAKE THIS
// virtual  void TurnTowardPosition(Vec2 const& targetPos, float maxTurnDegrees); 

    Map*          m_map                      = nullptr;
    EntityType    m_type                     = ENTITY_TYPE_UNKNOWN;
    EntityFaction m_faction                  = ENTITY_FACTION_UNKNOWN;
    Vec2          m_position                 = Vec2::ZERO;
    Vec2          m_velocity                 = Vec2::ZERO;
    float         m_orientationDegrees       = 0.f;
    float         m_targetOrientationDegrees = 0.f;
    float         m_physicsRadius            = 0.f;
    float         m_timeSinceLastRoll        = 0.f;
    int           m_health                   = 0;
    bool          m_isDead                   = false;
    bool          m_isGarbage                = false;
    bool          m_isPushedByEntities       = false;
    bool          m_doesPushEntities         = false;
    bool          m_isPushedByWalls            = false;
};
