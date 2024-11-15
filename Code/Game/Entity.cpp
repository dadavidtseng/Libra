//----------------------------------------------------------------------------------------------------
// Entity.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Entity.hpp"

#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------
Entity::Entity(Map* map, EntityType const type, EntityFaction const faction)
    : m_map(map),
      m_type(type),
      m_faction(faction)
{
}

//----------------------------------------------------------------------------------------------------
Vec2& Entity::GetPosition()
{
    return m_position;
}

//----------------------------------------------------------------------------------------------------
void Entity::SetPosition(const Vec2& newPosition)
{
    m_position = newPosition;
}

//----------------------------------------------------------------------------------------------------
float Entity::GetPhysicsRadius() const
{
    return m_physicsRadius;
}

//----------------------------------------------------------------------------------------------------
void Entity::TurnToward(float&      orientationDegrees,
                        const float targetOrientationDegrees,
                        const float deltaSeconds,
                        const float rotationSpeed)
{
    // Calculate the new target orientation and get the shortest angular displacement
    const float newTargetOrientation = GetTurnedTowardDegrees(orientationDegrees, targetOrientationDegrees,
                                                              rotationSpeed * deltaSeconds);

    // Update the orientation
    orientationDegrees = newTargetOrientation;

    // Normalize orientationDegrees to (0, 360)
    if (orientationDegrees >= 360.0f)
        orientationDegrees -= 360.0f;
    if (orientationDegrees < 0.0f)
        orientationDegrees += 360.0f;
}

void Entity::MoveToward(Vec2& currentPosition, Vec2 const& targetPosition, float const speed, float const deltaSeconds)
{
    Vec2 const  direction        = targetPosition - currentPosition;
    float const distanceToTarget = direction.GetLength();

    if (distanceToTarget > 0.0f)
    {
        Vec2 const  normalizedDirection = direction.GetNormalized();
        float const distanceToMove      = speed * deltaSeconds;

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
