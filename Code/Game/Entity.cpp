//----------------------------------------------------------------------------------------------------
// Entity.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Entity.hpp"

#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerTank.hpp"

//----------------------------------------------------------------------------------------------------
Entity::Entity(Map* map, EntityType const type, EntityFaction const faction)
    : m_map(map),
      m_type(type),
      m_faction(faction)
{
}

//----------------------------------------------------------------------------------------------------
void Entity::TurnToward(float&      orientationDegrees,
                        float const targetOrientationDegrees,
                        float const deltaSeconds,
                        float const rotationSpeed)
{
    // Calculate the new target orientation and get the shortest angular displacement
    orientationDegrees = GetTurnedTowardDegrees(orientationDegrees,
                                                targetOrientationDegrees,
                                                rotationSpeed * deltaSeconds);
}

//----------------------------------------------------------------------------------------------------
void Entity::MoveToward(Vec2&       currentPosition,
                        Vec2 const& targetPosition,
                        float const moveSpeed,
                        float const deltaSeconds)
{
    Vec2 const  direction        = targetPosition - currentPosition;
    float const distanceToTarget = direction.GetLength();

    if (distanceToTarget > 0.0f)
    {
        Vec2 const  normalizedDirection = direction.GetNormalized();
        float const distanceToMove      = moveSpeed * deltaSeconds;

        if (distanceToMove >= distanceToTarget)
        {
            // If the distance to move is greater than or equal to the distance to the target,
            // move directly to the target position.
            currentPosition = targetPosition;
        }
        else
        {
            // Otherwise, move towards the target position.
            currentPosition = currentPosition + normalizedDirection * distanceToMove;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Entity::WanderAround(float const deltaSeconds,
                          float const moveSpeed,
                          float const rotateSpeed)
{
    Vec2 const fwdNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees);

    if (m_timeSinceLastRoll >= 1.0f)
    {
        m_targetOrientationDegrees = static_cast<float>(g_theRNG->RollRandomIntInRange(0, 360));
        m_timeSinceLastRoll        = 0.f;
    }

    TurnToward(m_orientationDegrees,
               m_targetOrientationDegrees,
               deltaSeconds,
               rotateSpeed);

    m_velocity = fwdNormal * moveSpeed * deltaSeconds;
    m_position += m_velocity;
}

//----------------------------------------------------------------------------------------------------
void Entity::FindNextWayPosition()
{
    if (!m_heatMap)
        return;

    IntVec2 const currentTileCoords = m_map->GetTileCoordsFromWorldPos(m_position);
    IntVec2       bestNeighbor      = currentTileCoords;
    float         lowestHeat        = m_heatMap->GetValueAtCoords(currentTileCoords);

    for (IntVec2 const& offset : { IntVec2(-1, 0), IntVec2(1, 0), IntVec2(0, -1), IntVec2(0, 1) })
    {
        IntVec2     neighbor = currentTileCoords + offset;
        float const heat     = m_heatMap->GetValueAtCoords(neighbor);

        if (heat < lowestHeat)
        {
            lowestHeat   = heat;
            bestNeighbor = neighbor;
        }
    }

    m_nextWayPosition = m_map->GetWorldPosFromTileCoords(bestNeighbor);
}

void Entity::UpdateBehavior(float const deltaSeconds, bool const isChasing)
{
    PlayerTank const* playerTank = g_theGame->GetPlayerTank();

    // Update or initialize the heat map and target position
    if (!m_heatMap ||
        (isChasing && m_targetLastKnownPosition != playerTank->m_position))
    {
        m_nextWayPosition = m_position;

        // Create a new heat map with high initial values
        m_heatMap = new TileHeatMap(m_map->GetMapDimension(), 999.f);

        IntVec2 targetCoords;

        if (isChasing)
        {
            // Chasing mode: Set the target to the player's current position
            m_targetLastKnownPosition = playerTank->m_position;
            targetCoords              = IntVec2(static_cast<int>(playerTank->m_position.x), static_cast<int>(playerTank->m_position.y));
        }
        else
        {
            // Wandering mode: Set a random traversable tile as the target
            IntVec2 const randomCoords = m_map->RollRandomTraversableTileCoords();
            targetCoords               = randomCoords;
            m_targetLastKnownPosition  = m_map->GetWorldPosFromTileCoords(randomCoords);
        }

        // Generate heat maps and distance fields for pathfinding
        m_map->GenerateHeatMaps(*m_heatMap);
        m_map->GenerateDistanceFieldToPosition(*m_heatMap, targetCoords);
    }

    // Check if the target position has been reached
    if (IsPointInsideDisc2D(m_targetLastKnownPosition, m_position, m_physicsRadius))
    {
        m_hasTarget = false;
        delete m_heatMap;
        m_heatMap = nullptr;
        return;
    }

    // Check if the next waypoint has been reached
    if (IsPointInsideDisc2D(m_nextWayPosition, m_position, m_physicsRadius))
    {
        FindNextWayPosition();
    }

    Vec2 const dispToTarget = m_nextWayPosition - m_position;

    m_targetOrientationDegrees = Atan2Degrees(dispToTarget.y, dispToTarget.x);

    TurnToward(m_orientationDegrees,
               m_targetOrientationDegrees,
               deltaSeconds,
               m_rotateSpeed);

    MoveToward(m_position,
               m_nextWayPosition,
               m_moveSpeed,
               deltaSeconds);
}
