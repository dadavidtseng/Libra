//----------------------------------------------------------------------------------------------------
// Entity.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Entity.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------
Entity::Entity(Map* map, EntityType const type, EntityFaction const faction)
    : m_map(map),
      m_type(type),
      m_faction(faction)
{
}

//----------------------------------------------------------------------------------------------------
void Entity::TurnToward(float&      orientationDegrees,
                        const float targetOrientationDegrees,
                        const float deltaSeconds,
                        const float rotationSpeed)
{
    // Calculate the new target orientation and get the shortest angular displacement
    orientationDegrees = GetTurnedTowardDegrees(orientationDegrees,
                                                              targetOrientationDegrees,
                                                              rotationSpeed * deltaSeconds);
}

void Entity::MoveToward(Vec2& currentPosition, Vec2 const& targetPosition, float const moveSpeed, float const deltaSeconds)
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
void Entity::WanderAround(float deltaSeconds, float moveSpeed, float rotateSpeed)
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
