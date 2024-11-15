//-----------------------------------------------------------------------------------------------
// Map.hpp
//

//-----------------------------------------------------------------------------------------------
#pragma once
#include <vector>

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"

//-----------------------------------------------------------------------------------------------
struct Vertex_PCU;

typedef std::vector<Entity*> EntityList;

//-----------------------------------------------------------------------------------------------
class Map
{
public:
    Map(int width, int height);
    ~Map();
    void TestSpriteDefinition();

    void Update(float deltaSeconds);
    void Render();
    void DebugRender() const;

    AABB2   GetTileBounds(IntVec2 const& tileCoords) const;
    AABB2   GetTileBounds(int tileIndex) const;
    IntVec2 GetTileCoordsForWorldPos(Vec2 const& worldPos) const;
    bool    HasLineOfSight(Vec2 const& posA, Vec2 const& posB, float maxDist);

    Entity*         SpawnNewEntity(EntityType type, EntityFaction faction, Vec2 const& position, float orientationDegrees);
    RaycastResult2D RaycastVsTiles(Ray2 const& ray) const;
    void            TestTileDefinition();

private:
    void    UpdateEntities(float deltaSeconds);
    void    RenderEntities() const;
    void    GenerateTiles();
    void    SetLShapedBarrier(int startX, int startY, int size, bool isBottomLeft);
    bool    IsEdgeTile(int x, int y) const;
    bool    IsRandomStoneTile(int x, int y) const;
    void    AddVertsForTile(const Tile& tile, std::vector<Vertex_PCU>& vertices);
    Entity* CreateNewEntity(EntityType type, EntityFaction faction);
    void    AddEntityToMap(Entity* entity, Vec2 const& position, const float orientationDegrees);
    void    RemoveEntityFromMap(Entity* entity);
    void    AddEntityToList(Entity* entity, EntityList& entityList);
    void    RemoveEntityFromList(const Entity* entity, EntityList& entityList);
    void    SpawnNewNPCs();

    bool IsBullet(const Entity* entity) const;
    bool IsAgent(const Entity* entity) const;

    void PushEntitiesOutOfWalls();
// void PushEntitiesOutOfSolidTiles( Entity& entity );
// void PushEntitiesOutOfTileIfSolid( Entity& entity, IntVec2 const& tileCoords );
    void PushEntitiesOutOfEachOther(EntityList& entityListA, EntityList& entityListB) const;
    bool IsTileBlocking(Vec2 const& posA, Vec2 const& posB) const;

    std::vector<Tile> m_tiles;       // created and be there forever
    EntityList        m_allEntities; // created and destroyed
    EntityList        m_entitiesByType[NUM_ENTITY_TYPES];
    EntityList        m_agentsByFaction[NUM_ENTITY_FACTIONS];
    EntityList        m_bulletsByFaction[NUM_ENTITY_FACTIONS];
    IntVec2           m_dimensions;
    const TileDefinition* m_tileDef = nullptr;
};
