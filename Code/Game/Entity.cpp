//----------------------------------------------------------------------------------------------------
// Entity.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Entity.hpp"

#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
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

// TODO: FIX if player's spawn position is at the bottom-left of tile
//----------------------------------------------------------------------------------------------------
void Entity::UpdateBehavior(float const deltaSeconds, bool const isChasing)
{
    PlayerTank const* playerTank = g_theGame->GetPlayerTank();

    // Update or initialize the heat map and target position
    if (!m_heatMap ||
        (m_hasTarget && m_goalPosition != playerTank->m_position))
    {
        // Create a new heat map with high initial values
        m_heatMap = new TileHeatMap(m_map->GetMapDimension(), 999.f);

        IntVec2 targetCoords;

        if (m_hasTarget)
        {
            // Chasing mode: Set the target to the player's current position
            m_goalPosition = playerTank->m_position;
            targetCoords = m_map->GetTileCoordsFromWorldPos(m_goalPosition);

            // Play discover sound if not already played
            if (!m_hasPlayedDiscoverSound)
            {
                g_theAudio->StartSound(g_theGame->GetEnemyDiscoverSoundID());
                m_hasPlayedDiscoverSound = true;
                printf("PLAYDISCOVERSOUN\n");
                printf("(%f, %f) (%f, %f)\n", m_goalPosition.x, m_goalPosition.y, playerTank->m_position.x, playerTank->m_position.y);
            }
        }
        else
        {
            // Wandering mode: Set a random traversable tile as the target
            IntVec2 const randomCoords = m_map->RollRandomTraversableTileCoords(*m_heatMap, IntVec2(m_position));
            targetCoords = randomCoords;
            m_goalPosition = m_map->GetWorldPosFromTileCoords(randomCoords);

            // Reset discover sound flag when switching to wandering mode
            m_hasPlayedDiscoverSound = false;
        }

        // Generate heat maps and distance fields for pathfinding
        m_pathPoints = m_map->GenerateEntityPathToGoal(*m_heatMap, m_position, m_goalPosition);
    }

    // If path is empty, regenerate path
    if (m_pathPoints.empty())
    {
        m_pathPoints = m_map->GenerateEntityPathToGoal(*m_heatMap, m_position, m_goalPosition);
    }

    // Path navigation logic
    if (m_pathPoints.size() >= 2)
    {
        Vec2 nextNextPosition = m_pathPoints[m_pathPoints.size() - 2];
        if (!m_map->RaycastHitsImpassable(m_position, nextNextPosition))
        {
            m_pathPoints.pop_back();
        }
    }

    // Remove current target if reached
    if (IsPointInsideDisc2D(m_pathPoints.back(), m_position, m_physicsRadius))
    {
        m_pathPoints.pop_back();
    }

    // If path is empty, choose a new target
    if (m_pathPoints.empty())
    {
        IntVec2 randomCoords = m_map->RollRandomTraversableTileCoords(*m_heatMap, IntVec2(m_position));
        m_goalPosition = m_map->GetWorldPosFromTileCoords(randomCoords);
        m_pathPoints = m_map->GenerateEntityPathToGoal(*m_heatMap, m_position, m_goalPosition);
        m_hasTarget = false;
        m_hasPlayedDiscoverSound = false; // Reset sound flag
    }

    // Set target to the last point in the path
    Vec2 nextPosition = m_pathPoints.back();
    Vec2 dispToTarget = nextPosition - m_position;

    // Rotate and move
    m_targetOrientationDegrees = Atan2Degrees(dispToTarget.y, dispToTarget.x);
    TurnToward(m_orientationDegrees, m_targetOrientationDegrees, deltaSeconds, m_rotateSpeed);
    MoveToward(m_position, nextPosition, m_moveSpeed, deltaSeconds);
}

void Entity::RenderHealthBar() const
{
    VertexList  verts;
    AABB2 const box = AABB2(Vec2(-0.5f, 0.5f), Vec2(0.5f, 0.6f));

    AddVertsForAABB2D(verts, box, Rgba8::WHITE);

    TransformVertexArrayXY3D(static_cast<int>(verts.size()), verts.data(),
                             1.0f, 0.f, m_position);

    VertexList  healthBarVerts;
    AABB2 const healthBarBox = AABB2(Vec2(-0.5f, 0.5f), Vec2(0.5f * ((float) m_health / (float) m_totalHealth), 0.6f));
    AddVertsForAABB2D(healthBarVerts, healthBarBox, Rgba8::RED);

    TransformVertexArrayXY3D(static_cast<int>(healthBarVerts.size()), healthBarVerts.data(),
                             1.0f, 0.f, m_position);

    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
    g_theRenderer->DrawVertexArray(static_cast<int>(healthBarVerts.size()), healthBarVerts.data());
}
