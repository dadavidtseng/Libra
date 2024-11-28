//----------------------------------------------------------------------------------------------------
// Map.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include <vector>

#include "MapDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Entity.hpp"
#include "Game/Tile.hpp"

//----------------------------------------------------------------------------------------------------
class TileHeatMap;
struct MapDefinition;
struct Vertex_PCU;

//-----------------------------------------------------------------------------------------------
class Map
{
public:
    void     DebugRenderTileIndex() const;
    explicit Map(MapDefinition const& mapDef);

    ~Map();

    // TODO:
// SetTileType(int tileX, int tileY, TileType tileType);
// SetTileTypeInTriangle();
// RaycastResult2D RaycastVsHeatMap(Ray2 const& ray) const;

    void Update(float deltaSeconds);
    void Render() const;
    void DebugRender() const;

    // Accessors (const methods)
    IntVec2 const GetTileCoordsFromWorldPos(Vec2 const& worldPos) const;
    Vec2 const    GetWorldPosFromTileCoords(IntVec2 const& tileCoords) const;
    IntVec2 const GetMapDimension() const { return m_dimensions; }
    IntVec2 const GetMapExitPosition() const { return m_exitPosition; }
    int           GetMapIndex() const { return m_mapDef->GetIndex(); }
    int           GetTileNums() const { return m_dimensions.x * m_dimensions.y; }
    AABB2         GetMapBound() const;

    // Mutators (non-const methods)
    Entity* SpawnNewEntity(EntityType type, EntityFaction faction, Vec2 const& position, float orientationDegrees);
    void    AddEntityToMap(Entity* entity, Vec2 const& position, float orientationDegrees);
    void    RemoveEntityFromMap(Entity* entity);

    // Helpers
    RaycastResult2D RaycastVsTiles(Ray2 const& ray) const;
    bool            HasLineOfSight(Vec2 const& startPos, Vec2 const& endPos, float sightRange) const;
    bool            IsTileSolid(IntVec2 const& tileCoords) const;
    bool            IsPointInSolid(Vec2 const& point) const;

private:
    void UpdateEntities(float deltaSeconds) const;
    void RenderTiles() const;
    void RenderEntities() const;
    void RenderTileHeatMap() const;
    void DebugRenderEntities() const;


    // Map-related
    void        GenerateAllTiles();
    void        GenerateLShapeTiles(int tileCoordX, int tileCoordY, int width, int height, bool isBottomLeft);
    void        GenerateExitPosTile();
    bool        IsEdgeTile(int x, int y) const;
    bool        IsTileCoordsInLShape(int x, int y) const;
    bool        IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const;
    bool        IsWorldPosOccupied(Vec2 const& position) const;
    AABB2 const GetTileBounds(IntVec2 const& tileCoords) const;
    AABB2 const GetTileBounds(int tileIndex) const;
    IntVec2     RollRandomTileCoords() const;

    // Heatmap-related
    void GenerateHeatMaps() const;
    void GenerateDistanceField(IntVec2 const& startCoords, float specialValue) const;

    // Entity-lifetime-related
    Entity* CreateNewEntity(EntityType type, EntityFaction faction);
    void    AddEntityToList(Entity* entity, EntityList& entityList);
    void    RemoveEntityFromList(Entity const* entity, EntityList& entityList);
    void    DeleteGarbageEntities();
    void    SpawnNewNPCs();
    bool    IsBullet(Entity const* entity) const;
    bool    IsAgent(Entity const* entity) const;

    // Entity-physic-related
    void PushEntitiesOutOfWalls();
    void PushEntityOutOfSolidTiles(Entity* entity);
    void PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords);
    void PushEntitiesOutOfEachOther(EntityList const& entityListA, EntityList const& entityListB) const;
    void CheckEntityVsEntityCollision(EntityList const& entityListA, EntityList const& entityListB);

    std::vector<Tile>    m_tiles;
    EntityList           m_allEntities;
    EntityList           m_entitiesByType[NUM_ENTITY_TYPES];
    EntityList           m_agentsByFaction[NUM_ENTITY_FACTIONS];
    EntityList           m_bulletsByFaction[NUM_ENTITY_FACTIONS];
    IntVec2              m_exitPosition = IntVec2::ZERO;
    IntVec2              m_dimensions;
    MapDefinition const* m_mapDef = nullptr;

    // MetaData management
    TileHeatMap* m_tileHeatMap = nullptr;
};
