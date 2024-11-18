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

    AABB2           GetTileBounds(IntVec2 const& tileCoords) const;
    AABB2           GetTileBounds(int tileIndex) const;
    IntVec2         GetTileCoordsFromWorldPos(Vec2 const& worldPos) const;
    Vec2            GetWorldPosFromTileCoords(IntVec2 const& tileCoords) const;
    Entity*         SpawnNewEntity(EntityType type, EntityFaction faction, Vec2 const& position, float orientationDegrees);
    bool            HasLineOfSight(Vec2 const& posA, Vec2 const& posB, float maxDist) const;
    RaycastResult2D RaycastVsTiles(Ray2 const& ray) const;
    bool            IsTileSolid(IntVec2 const& tileCoords) const;
    bool            IsPointInSolid(Vec2 const& point) const;
    void            AddEntityToMap(Entity* entity, Vec2 const& position, float orientationDegrees);
    void            RemoveEntityFromMap(Entity* entity);
    IntVec2         GetMapDimension() { return m_dimensions; }
    int             GetMapIndex() const { return m_mapData.m_index; }
    IntVec2         GetMapExitPosition() { return m_exitPosition; }

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
    bool IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const;

    // Entity-lifetime-related
    Entity* CreateNewEntity(EntityType type, EntityFaction faction);

    void AddEntityToList(Entity* entity, EntityList& entityList);
    void RemoveEntityFromList(const Entity* entity, EntityList& entityList);
    void DeleteGarbageEntities();
    void SpawnNewNPCs();
    bool IsBullet(const Entity* entity) const;
    bool IsAgent(const Entity* entity) const;

    // Entity-physic-related
    void PushEntitiesOutOfWalls();
    void PushEntityOutOfSolidTiles(Entity* entity);
    void PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords);
    void PushEntitiesOutOfEachOther(EntityList const& entityListA, EntityList const& entityListB) const;
    void CheckEntityVsEntityCollision(EntityList const& entityListA, EntityList const& entityListB);

    std::vector<Tile> m_tiles;
    EntityList        m_allEntities;
    EntityList        m_entitiesByType[NUM_ENTITY_TYPES];
    EntityList        m_agentsByFaction[NUM_ENTITY_FACTIONS];
    EntityList        m_bulletsByFaction[NUM_ENTITY_FACTIONS];
    IntVec2           m_dimensions;
    IntVec2           m_exitPosition = IntVec2::ZERO;
    MapData           m_mapData;
};
