//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"
//----------------------------------------------------------------------------------------------------


#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Aries.hpp"
#include "Game/Bullet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Leo.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Scorpio.hpp"

//----------------------------------------------------------------------------------------------------
Map::Map(MapData const& data)
    : m_dimensions(data.m_dimensions.x, data.m_dimensions.y)
{
    m_tiles.reserve(static_cast<size_t>(m_dimensions.x) * static_cast<size_t>(m_dimensions.y));

    GenerateTiles();
    SpawnNewNPCs();
}

//----------------------------------------------------------------------------------------------------
Map::~Map()
{
    m_allEntities.clear();
    m_tiles.clear();
}

//----------------------------------------------------------------------------------------------------
void Map::Update(const float deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    UpdateEntities(deltaSeconds);

    PushEntitiesOutOfEntities(m_entitiesByType[ENTITY_TYPE_PLAYER_TANK], m_entitiesByType[ENTITY_TYPE_SCORPIO]);
    PushEntitiesOutOfEntities(m_entitiesByType[ENTITY_TYPE_LEO], m_entitiesByType[ENTITY_TYPE_SCORPIO]);
    PushEntitiesOutOfEntities(m_entitiesByType[ENTITY_TYPE_ARIES], m_entitiesByType[ENTITY_TYPE_SCORPIO]);
    PushEntitiesOutOfEachOther(m_entitiesByType[ENTITY_TYPE_PLAYER_TANK], m_entitiesByType[ENTITY_TYPE_LEO]);
    PushEntitiesOutOfEachOther(m_entitiesByType[ENTITY_TYPE_PLAYER_TANK], m_entitiesByType[ENTITY_TYPE_ARIES]);
    PushEntitiesOutOfEachOther(m_entitiesByType[ENTITY_TYPE_LEO], m_entitiesByType[ENTITY_TYPE_LEO]);
    PushEntitiesOutOfEachOther(m_entitiesByType[ENTITY_TYPE_ARIES], m_entitiesByType[ENTITY_TYPE_ARIES]);

    if (g_theGame->IsNoClip())
        return;

    PushEntitiesOutOfWalls();
}


//----------------------------------------------------------------------------------------------------
void Map::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderTiles();
    RenderEntities();
}


//----------------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
    if (!g_theGame->IsDebugRendering())
        return;

    DebugRenderEntities();
}

//----------------------------------------------------------------------------------------------------
AABB2 Map::GetTileBounds(IntVec2 const& tileCoords) const
{
    // 檢查座標是否在地圖範圍內
    if (tileCoords.x < 0 || tileCoords.x >= m_dimensions.x || tileCoords.y < 0 || tileCoords.y >= m_dimensions.y)
    {
        return {};
    }

    // 根據 tile 的座標計算其左下角和右上角
    const float minsX = static_cast<float>(tileCoords.x);
    const float minsY = static_cast<float>(tileCoords.y);
    const Vec2  mins(minsX, minsY);
    const Vec2  maxs(minsX + 1.0f, minsY + 1.0f);

    return AABB2(mins, maxs);
}

//----------------------------------------------------------------------------------------------------
AABB2 Map::GetTileBounds(const int tileIndex) const
{
    // 確保索引在範圍內
    if (tileIndex < 0 || tileIndex >= static_cast<int>(m_tiles.size()))
    {
        return {}; // 返回默認的 AABB2
    }

    // 將一維索引轉換為二維網格座標
    int     tileX = tileIndex % m_dimensions.x;
    int     tileY = tileIndex / m_dimensions.x;
    IntVec2 tileCoords(tileX, tileY);

    return GetTileBounds(tileCoords);
}

//----------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsFromWorldPos(Vec2 const& worldPos) const
{
    int tileX = static_cast<int>(floorf(worldPos.x));
    int tileY = static_cast<int>(floorf(worldPos.y));

    // 檢查是否在地圖範圍內
    if (tileX < 0 || tileX >= m_dimensions.x || tileY < 0 || tileY >= m_dimensions.y)
    {
        return IntVec2(-1, -1); // 如果不在範圍內，返回無效座標
    }

    return IntVec2(tileX, tileY);
}

Vec2 Map::GetWorldPosFromTileCoords(IntVec2 const& tileCoords) const
{
    // 檢查是否在地圖範圍內
    if (tileCoords.x < 0 || tileCoords.x >= m_dimensions.x ||
        tileCoords.y < 0 || tileCoords.y >= m_dimensions.y)
    {
        return Vec2(-1.f, -1.f); // 如果不在範圍內，返回無效座標
    }

    // 假設瓦片大小為 1x1，計算中心點位置
    float worldX = static_cast<float>(tileCoords.x) + 0.5f;
    float worldY = static_cast<float>(tileCoords.y) + 0.5f;

    return Vec2(worldX, worldY);
}

bool Map::HasLineOfSight(Vec2 const& posA, Vec2 const& posB, float const maxDist) const
{
    float const distSquared = GetDistanceSquared2D(posA, posB);

    // 檢查距離是否超過最大距離
    if (distSquared >= (maxDist * maxDist))
    {
        return false;
    }

    // 創建一個從 posA 到 posB 的射線

    const Vec2  fwdNormal   = (posB - posA).GetNormalized();
    const float maxDistance = GetDistance2D(posA, posB);
    const Ray2  ray         = Ray2(posA, fwdNormal, maxDistance);
    // 有障礙物阻擋

    return !RaycastVsTiles(ray).m_didImpact;
}

// // step-and-sample (slow, dumb, inaccurate, but really easy to write)
// // RaycastResult2D Map::RaycastVsTiles(Ray2 const& ray) const;

//----------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntity(const EntityType type, const EntityFaction faction, const Vec2& position,
                            const float      orientationDegrees)
{
    Entity* newEntity = CreateNewEntity(type, faction);
    AddEntityToMap(newEntity, position, orientationDegrees);

    return newEntity;
}

//----------------------------------------------------------------------------------------------------
void Map::UpdateEntities(const float deltaSeconds)
{
    for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); ++entityIndex)
    {
        if (Entity* entity = m_allEntities[entityIndex])
        {
            entity->Update(deltaSeconds);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
    for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); ++entityIndex)
    {
        if (const Entity* entity = m_allEntities[entityIndex])
        {
            entity->Render();
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
    for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); ++entityIndex)
    {
        if (const Entity* entity = m_allEntities[entityIndex])
        {
            entity->DebugRender();
        }
    }
}

//----------------------------------------------------------------------------------------------------
// Generate
void Map::GenerateTiles()
{
    for (int y = 0; y < m_dimensions.y; ++y)
    {
        for (int x = 0; x < m_dimensions.x; ++x)
        {
            TileType type = TILE_TYPE_GRASS;

            if (IsEdgeTile(x, y) ||
                IsRandomStoneTile(x, y))
            {
                type = TILE_TYPE_STONE;
            }

            m_tiles.emplace_back();
            m_tiles.back().m_tileCoords = IntVec2(x, y);
            m_tiles.back().m_type       = type;
        }
    }

    SetLShapedBarrier(2, 2, 5, false);
    SetLShapedBarrier(m_dimensions.x - 9, m_dimensions.y - 9, 7, true);
}

//----------------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
    std::vector<Vertex_PCU> tileVertices;

    RenderTilesByType(TILE_TYPE_GRASS, tileVertices);
    RenderTilesByType(TILE_TYPE_STONE, tileVertices);

    g_theRenderer->DrawVertexArray(static_cast<int>(tileVertices.size()), tileVertices.data());
}

//----------------------------------------------------------------------------------------------------
void Map::RenderTilesByType(TileType const tileType, VertexList& tileVertices) const
{
    TileDefinition const*  tileDef   = &TileDefinition::GetTileDefinition(tileType);
    SpriteDefinition const spriteDef = tileDef->GetSpriteDefinition();

    Vec2 const uvAtMins = spriteDef.GetUVsMins();
    Vec2 const uvAtMaxs = spriteDef.GetUVsMaxs();

    for (Tile const& tile : m_tiles)
    {
        if (tile.m_type == tileType)
        {
            Vec2 const mins(static_cast<float>(tile.m_tileCoords.x), static_cast<float>(tile.m_tileCoords.y));
            Vec2 const maxs = mins + Vec2(1.0f, 1.0f);

            AddVertsForAABB2D(tileVertices, AABB2(mins, maxs), tileDef->GetTintColor(), uvAtMins, uvAtMaxs);
            TransformVertexArrayXY3D(static_cast<int>(tileVertices.size()), tileVertices.data(), 1.0f, 0, Vec2::ZERO);
        }
    }

    g_theRenderer->BindTexture(&spriteDef.GetTexture());
}

//----------------------------------------------------------------------------------------------------
void Map::SetLShapedBarrier(int startX, int startY, int size, bool isBottomLeft)
{
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int const tileIndex = (startY + y) * m_dimensions.x + (startX + x);

            if (isBottomLeft)
            {
                if (y == 0 || x == 0)
                {
                    m_tiles[tileIndex].m_type = TILE_TYPE_STONE;
                }
            }
            else
            {
                if (y == size - 1 || x == size - 1)
                {
                    m_tiles[tileIndex].m_type = TILE_TYPE_STONE;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
bool Map::IsEdgeTile(int x, int y) const
{
    return
        x == 0 ||
        x == m_dimensions.x - 1 ||
        y == 0 ||
        y == m_dimensions.y - 1;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsRandomStoneTile(int x, int y) const
{
    const bool inLeftLShapeOpening  = x <= 7 && y <= 7;
    const bool inRightLShapeOpening = x >= m_dimensions.x - 9 && y >= m_dimensions.y - 9;

    return inLeftLShapeOpening || inRightLShapeOpening ? false : g_theRNG->RollRandomFloatZeroToOne() < 0.1f;
}

bool Map::IsTileSolid(IntVec2 const& tileCoords) const
{
    if (IsTileCoordsOutOfBounds(tileCoords))
    {
        return true; // Consider out-of-bounds as solid
    }

    int tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;
    if (tileIndex >= 0 && tileIndex < static_cast<int>(m_tiles.size()))
    {
        Tile const& tile = m_tiles[tileIndex];
        return tile.m_type == TILE_TYPE_STONE;
    }

    return false;
}

bool Map::IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const
{
    return
        tileCoords.x < 0 ||
        tileCoords.x >= m_dimensions.x ||
        tileCoords.y < 0 ||
        tileCoords.y >= m_dimensions.y;
}

//----------------------------------------------------------------------------------------------------
// choose what are we going to spawn and what faction it is
Entity* Map::CreateNewEntity(EntityType const type, EntityFaction const faction)
{
    switch (type)
    {
        case ENTITY_TYPE_PLAYER_TANK:
            return new PlayerTank(this, type, faction);
        case ENTITY_TYPE_SCORPIO:
            return new Scorpio(this, type, faction);
        case ENTITY_TYPE_LEO:
            return new Leo(this, type, faction);
        case ENTITY_TYPE_ARIES:
            return new Aries(this, type, faction);
        case ENTITY_TYPE_BULLET:
            return new Bullet(this, type, faction);
        case ENTITY_TYPE_UNKNOWN:
            ERROR_AND_DIE(Stringf("Unknown entity type #%i\n", type))
        default:
            ERROR_AND_DIE(Stringf("Unknown entity type #%i\n", type))
    }
}

void Map::AddEntityToMap(Entity* entity, Vec2 const& position, const float orientationDegrees)
{
    entity->m_map                = this;
    entity->m_position           = position;
    entity->m_orientationDegrees = orientationDegrees;

    AddEntityToList(entity, m_allEntities);
    AddEntityToList(entity, m_entitiesByType[entity->m_type]);

    if (IsBullet(entity))
    {
        AddEntityToList(entity, m_bulletsByFaction[entity->m_faction]);
    }

    if (IsAgent(entity))
    {
        AddEntityToList(entity, m_agentsByFaction[entity->m_faction]);
    }
}

void Map::AddEntityToList(Entity* entity, EntityList& entityList)
{
    entityList.push_back(entity);
}

//----------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromMap(Entity* entity)
{
    RemoveEntityFromList(entity, m_allEntities);
    RemoveEntityFromList(entity, m_entitiesByType[entity->m_type]);

    if (IsAgent(entity))
    {
        RemoveEntityFromList(entity, m_agentsByFaction[entity->m_faction]);
    }

    if (IsBullet(entity))
    {
        RemoveEntityFromList(entity, m_bulletsByFaction[entity->m_faction]);
    }

    entity->m_map = nullptr;
}

//----------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromList(Entity const* entity, EntityList& entityList)
{
    for (std::vector<Entity*>::iterator it = entityList.begin(); it != entityList.end(); ++it)
    {
        if (*it == entity)
        {
            entityList.erase(it);
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::SpawnNewNPCs()
{
    SpawnNewEntity(ENTITY_TYPE_SCORPIO, ENTITY_FACTION_EVIL, Vec2(4.5f, 5.5f), 0.f);
    SpawnNewEntity(ENTITY_TYPE_LEO, ENTITY_FACTION_EVIL, Vec2(2.5f, 7.5f), 0.f);
    SpawnNewEntity(ENTITY_TYPE_ARIES, ENTITY_FACTION_EVIL, Vec2(7.5f, 5.5f), 0.f);
}

//----------------------------------------------------------------------------------------------------
bool Map::IsBullet(Entity const* entity) const
{
    return
        entity->m_type == ENTITY_TYPE_BULLET;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsAgent(Entity const* entity) const
{
    return
        entity->m_type != ENTITY_TYPE_BULLET &&
        entity->m_type != ENTITY_TYPE_PLAYER_TANK;
}

//----------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfWalls()
{
    for (Entity* entity : m_allEntities)
    {
        PushEntityOutOfSolidTiles(entity);
    }
}

//----------------------------------------------------------------------------------------------------
void Map::PushEntityOutOfSolidTiles(Entity* entity)
{
    IntVec2 const myTileCoords = GetTileCoordsFromWorldPos(entity->m_position);

    // Push out of cardinal neighbors (NSEW) first 
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(1, 0));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(0, 1));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(-1, 0));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(0, -1));

    // Push out of diagonal neighbors second
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(1, 1));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(-1, 1));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(-1, -1));
    PushEntityOutOfTileIfSolid(entity, myTileCoords + IntVec2(1, -1));
}

//----------------------------------------------------------------------------------------------------
void Map::PushEntityOutOfTileIfSolid(Entity* entity, IntVec2 const& tileCoords)
{
    AABB2 const aabb2Box  = GetTileBounds(tileCoords);
    int const   tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;
    Tile const& tile      = m_tiles[tileIndex];

    if (tile.m_type == TILE_TYPE_GRASS)
        return;

    if (IsBullet(entity))
    {
        // Vec2 nearestPoint                  = aabb2Box.GetNearestPoint(entity->m_position);
        // Vec2 normalOfSurfaceToReflectOffOf = (entity->m_position - nearestPoint).GetNormalized();
        //
        // entity->m_velocity.Reflect(normalOfSurfaceToReflectOffOf);

        Vec2 const            fwdNormal       = Vec2::MakeFromPolarDegrees(entity->m_orientationDegrees);
        Ray2 const            ray             = Ray2(entity->m_position, fwdNormal.GetNormalized(), 0.5f);
        RaycastResult2D const raycastResult2D = RaycastVsTiles(ray);

        if (raycastResult2D.m_didImpact)
        {
            IntVec2 const myTileCoords = GetTileCoordsFromWorldPos(entity->m_position);

            Vec2 normalOfSurfaceToReflectOffOf = Vec2(static_cast<float>(tileCoords.x - myTileCoords.x), static_cast<float>(tileCoords.y - myTileCoords.y));

            entity->m_orientationDegrees = Atan2Degrees(entity->m_velocity.GetReflected(normalOfSurfaceToReflectOffOf).y,
                                                        entity->m_velocity.GetReflected(normalOfSurfaceToReflectOffOf).x);

            // RemoveEntityFromMap(entity);
            return;
        }
    }

    PushDiscOutOfAABB2D(entity->m_position, entity->m_physicsRadius, aabb2Box);
}

//----------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfEachOther(EntityList const& entityListA, EntityList const& entityListB) const
{
    for (Entity* entityA : entityListA)
    {
        if (!entityA)
            continue;

        for (Entity* entityB : entityListB)
        {
            if (!entityB)
                continue;

            if (entityA == entityB)
                continue;

            PushDiscsOutOfEachOther2D(entityA->m_position,
                                      entityA->m_physicsRadius,
                                      entityB->m_position,
                                      entityB->m_physicsRadius);
        }
    }
}
void Map::PushEntitiesOutOfEntities(EntityList const& entityListA, EntityList const& entityListB) const
{
    for (Entity* entityA : entityListA)
    {
        if (!entityA)
            continue;

        for (const Entity* entityB : entityListB)
        {
            if (!entityB)
                continue;

            PushDiscOutOfDisc2D(entityA->m_position,
                                entityA->m_physicsRadius,
                                entityB->m_position,
                                entityB->m_physicsRadius);
        }
    }
}

//----------------------------------------------------------------------------------------------------
RaycastResult2D Map::RaycastVsTiles(Ray2 const& ray) const
{
    RaycastResult2D raycastResult;
    raycastResult.m_rayFwdNormal = ray.m_direction;
    raycastResult.m_rayStartPos  = ray.m_origin;
    raycastResult.m_rayMaxLength = ray.m_maxDist;
    raycastResult.m_didImpact    = false;

    Vec2 currentPos = ray.m_origin;

    constexpr float stepSize = 0.01f;

    // Calculate the number of steps needed
    const int numSteps = static_cast<int>(ray.m_maxDist / stepSize);

    for (int i = 0; i < numSteps; ++i)
    {
        const float t = static_cast<float>(i) * stepSize;
        currentPos    = ray.m_origin + ray.m_direction * t;

        IntVec2 tileCoords = GetTileCoordsFromWorldPos(currentPos);

        // Check bounds
        if (tileCoords.x < 0 || tileCoords.x >= m_dimensions.x ||
            tileCoords.y < 0 || tileCoords.y >= m_dimensions.y)
        {
            raycastResult.m_didImpact  = true;
            raycastResult.m_impactDist = t;
            raycastResult.m_impactPos  = currentPos;
            return raycastResult; // Out of bounds is considered blocking
        }

        // Check tile blocking
        int tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;
        if (tileIndex >= 0 && tileIndex < static_cast<int>(m_tiles.size()))
        {
            Tile const& tile = m_tiles[tileIndex];
            if (tile.m_type == TILE_TYPE_STONE)
            {
                raycastResult.m_didImpact  = true;
                raycastResult.m_impactDist = t;
                raycastResult.m_impactPos  = currentPos;

                return raycastResult;
            }
        }
    }

    return raycastResult;
}
