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
    PushEntitiesOutOfWalls();

// PushEntitiesOutOfEachOther();

    if (g_theGame->IsNoClip())
        return;

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
        return AABB2(); // 返回默認的 AABB2
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
    for (int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType)
    {
        const EntityList& entitiesOfType = m_entitiesByType[entityType];

        for (int entityIndex = 0; entityIndex < static_cast<int>(entitiesOfType.size()); ++entityIndex)
        {
            if (Entity* entity = entitiesOfType[entityIndex])
            {
                entity->Update(deltaSeconds);
            }
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

//----------------------------------------------------------------------------------------------------
// choose what are we going to spawn and what faction it is
Entity* Map::CreateNewEntity(EntityType const type, EntityFaction const faction)
{
    switch (type)
    {
        case ENTITY_TYPE_PLAYER_TANK:
            return new PlayerTank(this, type, faction);
        case ENTITY_SCORPIO:
            return new Scorpio(this, type, faction);
        case ENTITY_LEO:
            return new Leo(this, type, faction);
        case ENTITY_ARIES:
            return new Aries(this, type, faction);
        case ENTITY_BULLET:
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

    if (IsBullet(entity)) { AddEntityToList(entity, m_bulletsByFaction[entity->m_faction]); }
    if (IsAgent(entity)) { AddEntityToList(entity, m_agentsByFaction[entity->m_faction]); }
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

    if (IsAgent(entity)) { RemoveEntityFromList(entity, m_agentsByFaction[entity->m_faction]); }

    if (IsBullet(entity)) { RemoveEntityFromList(entity, m_bulletsByFaction[entity->m_faction]); }

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
    SpawnNewEntity(ENTITY_SCORPIO, ENTITY_FACTION_EVIL, Vec2(4.5f, 5.5f), 0.f);
    SpawnNewEntity(ENTITY_LEO, ENTITY_FACTION_EVIL, Vec2(2.5f, 7.5f), 0.f);
    SpawnNewEntity(ENTITY_ARIES, ENTITY_FACTION_EVIL, Vec2(7.5f, 5.5f), 0.f);
}

//----------------------------------------------------------------------------------------------------
bool Map::IsBullet(Entity const* entity) const
{
    return
        entity->m_type == ENTITY_BULLET;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsAgent(Entity const* entity) const
{
    return
        entity->m_type != ENTITY_BULLET &&
        entity->m_type != ENTITY_TYPE_PLAYER_TANK;
}

//----------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfWalls()
{
    for (Entity* entity : m_allEntities)
    {
        const Vec2    playerPos        = entity->m_position;
        const IntVec2 playerTileCoords = GetTileCoordsFromWorldPos(playerPos);

        const IntVec2 offsets[] =
        {
            IntVec2(0, 1), IntVec2(0, -1), IntVec2(1, 0), IntVec2(-1, 0),
            IntVec2(1, 1), IntVec2(1, -1), IntVec2(-1, 1), IntVec2(-1, -1)
        };

        for (int i = 0; i < 4; ++i)
        {
            IntVec2 offset         = offsets[i];
            IntVec2 neighborCoords = playerTileCoords + offset;

            AABB2 neighborBox = GetTileBounds(neighborCoords);

            // 找到 tileIndex 後檢查是否在地圖內
            int const tileIndex = neighborCoords.y * m_dimensions.x + neighborCoords.x;

            if (tileIndex < 0 ||
                tileIndex >= static_cast<int>(m_tiles.size()))
                continue;

            // 檢查 tile 類型
            const Tile& tile = m_tiles[tileIndex];

            if (tile.m_type == TILE_TYPE_GRASS)
                continue;

            PushDiscOutOfAABB2D(entity->m_position, entity->m_physicsRadius, neighborBox);
        }

        // 接著處理 diagonal (NE/NW/SE/SW) 方向上的相鄰 tiles
        for (int i = 4; i < 8; ++i) // Diagonal directions: 4, 5, 6, 7 (右上、右下、左上、左下)
        {
            IntVec2 offset         = offsets[i];
            IntVec2 neighborCoords = playerTileCoords + offset;

            // 使用 GetTileBounds 取得相鄰 tile 的邊界（AABB2）
            AABB2 fixedBox = GetTileBounds(neighborCoords);

            // 找到 tileIndex 後檢查是否在地圖內
            int const tileIndex = neighborCoords.y * m_dimensions.x + neighborCoords.x;

            if (tileIndex < 0 ||
                tileIndex >= static_cast<int>(m_tiles.size()))
                continue;

            // 檢查 tile 類型
            const Tile& tile = m_tiles[tileIndex];

            if (tile.m_type == TILE_TYPE_GRASS)
                continue;

            PushDiscOutOfAABB2D(entity->m_position, entity->m_physicsRadius, fixedBox);
        }
    }
}

//Player vs Enemy
//Player vs Bullet  //DO NOT PUSH
//Enemy vs Bullet   //DO NOT PUSH
//Enemy vs Enemy

//Entity vs Wall

//----------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfEachOther(EntityList& entityListA, EntityList& entityListB) const
{
    for (size_t i = 0; i < m_allEntities.size(); ++i)
    {
        Entity* entityA = m_allEntities[i];

        if (entityA == nullptr)
            continue;

        for (size_t j = i + 1; j < m_allEntities.size(); ++j)
        {
            Entity* entityB = m_allEntities[j];

            if (entityB == nullptr)
                continue;

            PushDiscsOutOfEachOther2D(entityA->m_position,
                                      entityA->m_physicsRadius,
                                      entityB->m_position,
                                      entityB->m_physicsRadius);
        }
    }
}

// void Map::PushEntitiesOutOfSolidTiles(Entity& entity)
// {
// }
// void Map::PushEntitiesOutOfTileIfSolid(Entity& entity, IntVec2 const& tileCoords)
// {
// }

//----------------------------------------------------------------------------------------------------
bool Map::IsTileBlocking(Vec2 const& posA, Vec2 const& posB) const
{
    constexpr float stepSize = 10.f;

    const Vec2  direction = posB - posA;
    const float distance  = direction.GetLength();

    // Calculate the number of steps needed
    const int numSteps = static_cast<int>(distance / stepSize);

    for (int i = 0; i <= numSteps; ++i)
    {
        // Calculate the current position using the integer loop counter
        const float   t          = static_cast<float>(i) * stepSize;
        Vec2          currentPos = posA + direction.GetNormalized() * t;
        const IntVec2 tileCoords = GetTileCoordsFromWorldPos(currentPos);

        if (tileCoords.x < 0 || tileCoords.x >= m_dimensions.x ||
            tileCoords.y < 0 || tileCoords.y >= m_dimensions.y)
        {
            return true; // Out of bounds is considered blocking
        }

        int tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;
        if (tileIndex >= 0 && tileIndex < static_cast<int>(m_tiles.size()))
        {
            const Tile& tile = m_tiles[tileIndex];

            if (tile.m_type == TileType::TILE_TYPE_STONE) { return true; }
        }
    }

    return false;
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
    // printf("numSteps: %d\n", numSteps);

    for (int i = 0; i < numSteps; ++i)
    {
        const float t = static_cast<float>(i) * stepSize;
        currentPos    = ray.m_origin + ray.m_direction * t;

        if (IsTileBlocking(currentPos, currentPos + ray.m_direction))
        {
            raycastResult.m_didImpact    = true;
            raycastResult.m_impactDist   = t;
            raycastResult.m_impactPos    = currentPos;
            raycastResult.m_impactNormal = -ray.m_direction; // Assuming opposite direction as normal
            // printf("%f, %f\n", ray.m_origin.x, ray.m_origin.y);
            // printf("%f, %f\n", ray.m_direction.x, ray.m_direction.y);
            // printf("current Pos: %f, %f\n", currentPos.x, currentPos.y);
            // printf("target Pos: %f, %f\n", (currentPos+ray.m_direction).x, (currentPos+ray.m_direction).y);
            // printf("%f\n", t);
            return raycastResult;
        }
    }

    return raycastResult;
}

// void Map::TestTileDefinition()
// {
//     m_tileDef = &TileDefinition::GetTileDefinition(TILE_TYPE_GRASS);
//
//     const SpriteDefinition grassSpriteDef = m_tileDef->GetSpriteDefinition();
//
//     Vec2 const uvAtMins = grassSpriteDef.GetUVsMins();
//     Vec2 const uvAtMaxs = grassSpriteDef.GetUVsMaxs();
//
//     std::vector<Vertex_PCU> testVerts;
//     AABB2 const             box = AABB2(Vec2(2, 2), Vec2(3, 3));
//     AddVertsForAABB2D(testVerts, box, Rgba8(255, 255, 255), uvAtMins, uvAtMaxs);
//
//     TransformVertexArrayXY3D(static_cast<int>(testVerts.size()), testVerts.data(), 1.0f, 0, Vec2(0, 0));
//
//     g_theRenderer->BindTexture(&grassSpriteDef.GetTexture());
//     g_theRenderer->DrawVertexArray(static_cast<int>(testVerts.size()), testVerts.data());
// }

// void Map::TestSpriteDefinition()
// {
//     Texture*                testTexture_8x2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestSpriteSheet_8x2.png");
//     SpriteSheet*            testSpriteSheet = new SpriteSheet(*testTexture_8x2, IntVec2(8, 2));
//     SpriteDefinition const& testSpriteDef   = testSpriteSheet->GetSpriteDef(0);
//
//     Vec2 uvAtMins = testSpriteDef.GetUVsMins();
//     Vec2 uvAtMaxs = testSpriteDef.GetUVsMaxs();
//
//     std::vector<Vertex_PCU> testVerts;
//     AABB2                   box = AABB2(Vec2(0, 0), Vec2(1, 1));
//     AddVertsForAABB2D(testVerts, box, Rgba8(255, 255, 255), uvAtMins, uvAtMaxs);
//
//     TransformVertexArrayXY3D(static_cast<int>(testVerts.size()), testVerts.data(), 1.0f, 0, Vec2(2, 0));
//
//     // Bind the whole texture (sprite sheet) but only render a specific part using UVs
//     g_theRenderer->BindTexture(&testSpriteDef.GetTexture());
//     g_theRenderer->DrawVertexArray(static_cast<int>(testVerts.size()), testVerts.data());
// }
