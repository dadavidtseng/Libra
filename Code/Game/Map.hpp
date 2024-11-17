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

struct MapData
{
    int     m_index;
    IntVec2 m_dimensions;
};

//-----------------------------------------------------------------------------------------------
class Map
{
public:
    explicit Map(MapData const& data);
    ~Map();

// TODO:
    // SetTileType(int tileX, int tileY, TileType tileType);
    // SetTileTypeInTriangle();
    void Update(float deltaSeconds);
    void Render() const;
    void DebugRender() const;

    AABB2   GetTileBounds(IntVec2 const& tileCoords) const;
    AABB2   GetTileBounds(int tileIndex) const;
    IntVec2 GetTileCoordsFromWorldPos(Vec2 const& worldPos) const;
    Vec2    GetWorldPosFromTileCoords(IntVec2 const& tileCoords) const;
    bool    HasLineOfSight(Vec2 const& posA, Vec2 const& posB, float maxDist) const;

    Entity*         SpawnNewEntity(EntityType type, EntityFaction faction, Vec2 const& position, float orientationDegrees);
    RaycastResult2D RaycastVsTiles(Ray2 const& ray) const;

private:
    void UpdateEntities(float deltaSeconds);
    void RenderEntities() const;
    void DebugRenderEntities() const;

    // Map-related
    void GenerateTiles();
    void RenderTiles() const;
    void RenderTilesByType(TileType tileType, std::vector<Vertex_PCU>& tileVertices) const;
    void SetLShapedBarrier(int startX, int startY, int size, bool isBottomLeft);
    bool IsEdgeTile(int x, int y) const;
    bool IsRandomStoneTile(int x, int y) const;
    bool IsTileSolid(IntVec2 const& tileCoords) const;
    bool IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const;

    Entity* CreateNewEntity(EntityType type, EntityFaction faction);
    void    AddEntityToMap(Entity* entity, Vec2 const& position, float orientationDegrees);
    void    RemoveEntityFromMap(Entity* entity);
    void    AddEntityToList(Entity* entity, EntityList& entityList);
    void    RemoveEntityFromList(const Entity* entity, EntityList& entityList);
    void    SpawnNewNPCs();

    bool IsBullet(const Entity* entity) const;
    bool IsAgent(const Entity* entity) const;

    void PushEntitiesOutOfWalls();
    void PushEntityOutOfSolidTiles(Entity* entity);
    void PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords);

    void PushEntitiesOutOfEachOther(EntityList const& entityListA, EntityList const& entityListB) const;
    void PushEntitiesOutOfEntities(EntityList const& entityListA, EntityList const& entityListB) const;

    std::vector<Tile> m_tiles;       // created and be there forever
    EntityList        m_allEntities; // created and destroyed
    EntityList        m_entitiesByType[NUM_ENTITY_TYPES];
    EntityList        m_agentsByFaction[NUM_ENTITY_FACTIONS];
    EntityList        m_bulletsByFaction[NUM_ENTITY_FACTIONS];
    IntVec2           m_dimensions;
};
