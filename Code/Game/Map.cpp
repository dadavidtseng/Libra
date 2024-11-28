//----------------------------------------------------------------------------------------------------
// Map.cpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#include "Game/Map.hpp"

#include <cmath>

#include "MapDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/HeatMaps.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Aries.hpp"
#include "Game/Bullet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Leo.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Scorpio.hpp"

void Map::DebugRenderTileIndex() const
{
    BitmapFont const* g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile("Data/Fonts/SquirrelFixedFont"); // DO NOT SPECIFY FILE .EXTENSION!!  (Important later on.)

    for (int tileY = 0; tileY < m_dimensions.y; ++tileY)
    {
        for (int tileX = 0; tileX < m_dimensions.x; ++tileX)
        {
            IntVec2 tileCoords(tileX, tileY);
            int     tileIndex = tileY * m_dimensions.x + tileX;
            float   value     = m_tileHeatMap->GetValueAtCoords(tileX, tileY);

            std::vector<Vertex_PCU> textVerts;
            g_testFont->AddVertsForText2D(textVerts, Vec2((float) tileX, (float) tileY), 0.2f, std::to_string(static_cast<int>(value)), Rgba8::BLACK);
            g_theRenderer->BindTexture(&g_testFont->GetTexture());
            g_theRenderer->DrawVertexArray(static_cast<int>(textVerts.size()), textVerts.data());
        }
    }
}

//----------------------------------------------------------------------------------------------------
Map::Map(MapDefinition const& mapDef)
    : m_mapDef(&mapDef)
{
    m_dimensions = mapDef.GetDimensions();
    m_tiles.reserve(static_cast<size_t>(m_dimensions.x) * static_cast<size_t>(m_dimensions.y));
    m_exitPosition = IntVec2(m_dimensions.x - 2, m_dimensions.y - 2);
    m_tileHeatMap  = new TileHeatMap(m_dimensions, 0.f);

    GenerateAllTiles();
    SpawnNewNPCs();
    GenerateHeatMaps();
    GenerateDistanceField(IntVec2::ONE, 999.f);

    // while (!wasSuccessful)
    // {
    //     PopulateTiles();
    // }
}

//----------------------------------------------------------------------------------------------------
Map::~Map()
{
    m_allEntities.clear();
    m_tiles.clear();
}

//----------------------------------------------------------------------------------------------------
void Map::Update(float const deltaSeconds)
{
    if (g_theGame->IsAttractMode())
        return;

    UpdateEntities(deltaSeconds);
    PushEntitiesOutOfEachOther(m_allEntities, m_allEntities);
    CheckEntityVsEntityCollision(m_entitiesByType[ENTITY_TYPE_BULLET], m_allEntities);
    PushEntitiesOutOfWalls();
    DeleteGarbageEntities();
}

//----------------------------------------------------------------------------------------------------
void Map::Render() const
{
    if (g_theGame->IsAttractMode())
        return;

    RenderTiles();
    RenderTileHeatMap();
    DebugRenderTileIndex();

    RenderEntities();
}

//----------------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
    if (g_theGame->IsAttractMode())
        return;

    if (!g_theGame->IsDebugRendering())
        return;

    DebugRenderEntities();
}

//----------------------------------------------------------------------------------------------------
IntVec2 const Map::GetTileCoordsFromWorldPos(Vec2 const& worldPos) const
{
    int const tileX = RoundDownToInt(worldPos.x);
    int const tileY = RoundDownToInt(worldPos.y);

    if (IsTileCoordsOutOfBounds(IntVec2(tileX, tileY)))
    {
        printf("%d, %d", tileX, tileY);
        ERROR_AND_DIE("tileCoords is out of bound")
    }

    return IntVec2(tileX, tileY);
}

//----------------------------------------------------------------------------------------------------
Vec2 const Map::GetWorldPosFromTileCoords(IntVec2 const& tileCoords) const
{
    if (IsTileCoordsOutOfBounds(tileCoords))
        ERROR_AND_DIE("tileCoords is out of bound")

    constexpr float halfTileWidth = 0.5f;
    float const     worldX        = static_cast<float>(tileCoords.x) + halfTileWidth;
    float const     worldY        = static_cast<float>(tileCoords.y) + halfTileWidth;

    return Vec2(worldX, worldY);
}

//----------------------------------------------------------------------------------------------------
int Map::GetNumTiles() const
{
    return m_dimensions.x * m_dimensions.y;
}

//----------------------------------------------------------------------------------------------------
AABB2 Map::GetMapBound() const
{
    return AABB2(IntVec2::ZERO, m_dimensions);
}

//----------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntity(EntityType const    type,
                            EntityFaction const faction,
                            Vec2 const&         position,
                            float const         orientationDegrees)
{
    Entity* newEntity = CreateNewEntity(type, faction);

    AddEntityToMap(newEntity, position, orientationDegrees);

    return newEntity;
}

//----------------------------------------------------------------------------------------------------
bool Map::HasLineOfSight(Vec2 const& startPos,
                         Vec2 const& endPos,
                         float const sightRange) const
{
    float const distSquared      = GetDistanceSquared2D(startPos, endPos);
    float const sighRangeSquared = sightRange * sightRange;

    if (distSquared >= sighRangeSquared)
        return false;

    Vec2 const  fwdNormal = (endPos - startPos).GetNormalized();
    float const maxDist   = GetDistance2D(startPos, endPos);
    Ray2 const  ray       = Ray2(startPos, fwdNormal, maxDist);

    return !RaycastVsTiles(ray).m_didImpact;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileSolid(IntVec2 const& tileCoords) const
{
    if (IsTileCoordsOutOfBounds(tileCoords))
        return true;

    int const tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;

    if (tileIndex >= 0 &&
        tileIndex < static_cast<int>(m_tiles.size()))
    {
        Tile const& tile = m_tiles[tileIndex];

        return tile.m_tileName == "Stone";
    }

    return false;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsPointInSolid(Vec2 const& point) const
{
    IntVec2 const tileCoords = GetTileCoordsFromWorldPos(point);

    return IsTileSolid(tileCoords);
}

//----------------------------------------------------------------------------------------------------
void Map::UpdateEntities(float const deltaSeconds) const
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
void Map::RenderTiles() const
{
    VertexList tileVertices;

    tileVertices.reserve(static_cast<size_t>(3) * 2 * m_dimensions.x * m_dimensions.y);

    for (String const& tileName : TileDefinition::GetTileNames())
    {
        TileDefinition const*  tileDef   = TileDefinition::GetTileDefByName(tileName);
        SpriteDefinition const spriteDef = tileDef->GetSpriteDef();

        Vec2 const uvAtMins = spriteDef.GetUVsMins();
        Vec2 const uvAtMaxs = spriteDef.GetUVsMaxs();

        for (Tile const& tile : m_tiles)
        {
            if (tile.m_tileName == tileName)
            {
                Vec2 const mins(static_cast<float>(tile.m_tileCoords.x), static_cast<float>(tile.m_tileCoords.y));
                Vec2 const maxs = mins + Vec2::ONE;

                AddVertsForAABB2D(tileVertices, AABB2(mins, maxs), tileDef->GetTintColor(), uvAtMins, uvAtMaxs);
            }
        }
    }

    g_theRenderer->BindTexture(&g_theGame->GetTileSpriteSheet()->GetTexture());
    g_theRenderer->DrawVertexArray(static_cast<int>(tileVertices.size()), tileVertices.data());
}

//----------------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
    for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); ++entityIndex)
    {
        if (Entity const* entity = m_allEntities[entityIndex])
        {
            entity->Render();
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::RenderTileHeatMap() const
{
    AABB2 const totalBounds = GetMapBound();

    VertexList verts;

    m_tileHeatMap->AddVertsForDebugDraw(verts, totalBounds);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
}

//----------------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
    for (int entityIndex = 0; entityIndex < static_cast<int>(m_allEntities.size()); ++entityIndex)
    {
        if (Entity const* entity = m_allEntities[entityIndex])

            entity->DebugRender();
    }
}

//----------------------------------------------------------------------------------------------------
void Map::GenerateAllTiles()
{
    printf("( Map%d ) Start  | GenerateAllTiles\n", m_mapDef->GetIndex());

    for (int y = 0; y < m_dimensions.y; ++y)
    {
        for (int x = 0; x < m_dimensions.x; ++x)
        {
            String tileName  = "Grass";
            int    tileIndex = 0;

            if (g_theRNG->RollRandomFloatZeroToOne() < 0.1f)
            {
                tileName  = "Sparkle_01";
                tileIndex = 2;
            }

            if (g_theRNG->RollRandomFloatZeroToOne() < 0.2f)
            {
                tileName  = "Sparkle_02";
                tileIndex = 3;
            }

            if (IsTileCoordsInLShape(x, y))
            {
                tileName  = "Floor";
                tileIndex = 4;
            }

            if (IsEdgeTile(x, y) ||
                (!IsTileCoordsInLShape(x, y) && g_theRNG->RollRandomFloatZeroToOne() < 0.1f))
            {
                tileName  = "Stone";
                tileIndex = 1;
            }

            m_tiles.emplace_back();
            m_tiles.back().m_tileCoords   = IntVec2(x, y);
            m_tiles.back().m_tileName     = tileName;
            m_tiles.back().m_tileDefIndex = tileIndex;
        }
    }

    GenerateLShapeTiles(2, 2, 5, 5, false);
    GenerateLShapeTiles(m_dimensions.x - 9, m_dimensions.y - 9, 7, 7, true);
    GenerateExitPosTile();

    printf("( Map%d ) Finish | GenerateAllTiles\n", m_mapDef->GetIndex());
}

//----------------------------------------------------------------------------------------------------
void Map::GenerateLShapeTiles(int const  tileCoordX,
                              int const  tileCoordY,
                              int const  width,
                              int const  height,
                              bool const isBottomLeft)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int const tileIndex = (tileCoordY + y) * m_dimensions.x + (tileCoordX + x);

            if (isBottomLeft)
            {
                if (y == 0 || x == 0)
                {
                    m_tiles[tileIndex].m_tileName     = "Stone";
                    m_tiles[tileIndex].m_tileDefIndex = 1;
                }
            }
            else
            {
                if (y == height - 1 || x == width - 1)
                {
                    m_tiles[tileIndex].m_tileName     = "Stone";
                    m_tiles[tileIndex].m_tileDefIndex = 1;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::GenerateExitPosTile()
{
    m_tiles.emplace_back();
    m_tiles.back().m_tileCoords   = m_exitPosition;
    m_tiles.back().m_tileName     = "Exit";
    m_tiles.back().m_tileDefIndex = 5;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsEdgeTile(int const x, int const y) const
{
    return
        x == 0 ||
        x == m_dimensions.x - 1 ||
        y == 0 ||
        y == m_dimensions.y - 1;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileCoordsInLShape(int const x, int const y) const
{
    bool const inLeftLShape  = x <= 5 && y <= 5;
    bool const inRightLShape = x >= m_dimensions.x - 8 && y >= m_dimensions.y - 8;

    return inLeftLShape || inRightLShape;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsTileCoordsOutOfBounds(IntVec2 const& tileCoords) const
{
    return
        tileCoords.x < 0 ||
        tileCoords.x >= m_dimensions.x ||
        tileCoords.y < 0 ||
        tileCoords.y >= m_dimensions.y;
}

//----------------------------------------------------------------------------------------------------
bool Map::IsWorldPosOccupied(Vec2 const& position) const
{
    for (Entity const* entity : m_allEntities)
    {
        if (entity->m_position == position)
        {
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------------------------------
AABB2 const Map::GetTileBounds(IntVec2 const& tileCoords) const
{
    if (IsTileCoordsOutOfBounds(tileCoords))
        ERROR_AND_DIE("tileCoords is out of bound")

    float const minsX = static_cast<float>(tileCoords.x);
    float const minsY = static_cast<float>(tileCoords.y);
    Vec2 const  mins(minsX, minsY);
    Vec2 const  maxs(mins + Vec2::ONE);

    return AABB2(mins, maxs);
}

//----------------------------------------------------------------------------------------------------
AABB2 const Map::GetTileBounds(int const tileIndex) const
{
    if (tileIndex < 0 || tileIndex >= static_cast<int>(m_tiles.size()))
        ERROR_AND_DIE("tileIndex is out of bound")

    int const     tileX = tileIndex % m_dimensions.x;
    int const     tileY = tileIndex / m_dimensions.x;
    IntVec2 const tileCoords(tileX, tileY);

    return GetTileBounds(tileCoords);
}

//----------------------------------------------------------------------------------------------------
IntVec2 Map::RollRandomTileCoords() const
{
    int const randomX = g_theRNG->RollRandomIntInRange(0, m_dimensions.x - 1);
    int const randomY = g_theRNG->RollRandomIntInRange(0, m_dimensions.y - 1);

    return IntVec2(randomX, randomY);
}

//----------------------------------------------------------------------------------------------------
void Map::GenerateHeatMaps() const
{
    printf("( Map%d ) Start  | GenerateHeatMaps\n", m_mapDef->GetIndex());

    int const numTiles = m_tileHeatMap->GetTileNums();
    for (int i = 0; i < numTiles; i++)
    {
        if (TileDefinition::s_tileDefinitions[m_tiles[i].m_tileDefIndex]->IsSolid())
        {
            m_tileHeatMap->SetValueAtIndex(i, 999.f);
        }
    }

    printf("( Map%d ) Finish | GenerateHeatMaps\n", m_mapDef->GetIndex());
}

//----------------------------------------------------------------------------------------------------
void Map::GenerateDistanceField(IntVec2 const& startCoords, float const specialValue) const
{
    printf("( Map%d ) Start  | GenerateDistanceField\n", m_mapDef->GetIndex());

    m_tileHeatMap->SetValueAtAllTiles(specialValue);
    m_tileHeatMap->SetValueAtCoords(startCoords, 0.f);

    float currentSearchValue = 0.f;
    bool  isStillGoing       = true;

    while (isStillGoing)// For each pass, assume we're done UNLESS something changes
    {
        isStillGoing = false;

        for (int tileY = 0; tileY < m_dimensions.y; ++tileY)
        {
            for (int tileX = 0; tileX < m_dimensions.x; ++tileX)
            {
                IntVec2     tileCoords(tileX, tileY);
                float const value = m_tileHeatMap->GetValueAtCoords(tileX, tileY);

                if (std::fabs(value - currentSearchValue) < EPSILON)
                {
                    // Found a search value ! Spread to cardinal neighbors...
                    IntVec2     n               = tileCoords + IntVec2(0, 1);
                    IntVec2     e               = tileCoords + IntVec2(1, 0);
                    IntVec2     w               = tileCoords + IntVec2(-1, 0);
                    IntVec2     s               = tileCoords + IntVec2(0, -1);
                    float const nextSearchValue = currentSearchValue + 1.f;

                    if (!IsTileCoordsOutOfBounds(e) && !IsTileSolid(e) && m_tileHeatMap->GetValueAtCoords(e) > nextSearchValue)
                    {
                        m_tileHeatMap->SetValueAtCoords(e, nextSearchValue);
                        isStillGoing = true;
                    }

                    if (!IsTileCoordsOutOfBounds(n) && !IsTileSolid(n) && m_tileHeatMap->GetValueAtCoords(n) > nextSearchValue)
                    {
                        m_tileHeatMap->SetValueAtCoords(n, nextSearchValue);
                        isStillGoing = true;
                    }

                    if (!IsTileCoordsOutOfBounds(s) && !IsTileSolid(s) && m_tileHeatMap->GetValueAtCoords(s) > nextSearchValue)
                    {
                        m_tileHeatMap->SetValueAtCoords(s, nextSearchValue);
                        isStillGoing = true;
                    }

                    if (!IsTileCoordsOutOfBounds(w) && !IsTileSolid(w) && m_tileHeatMap->GetValueAtCoords(w) > nextSearchValue)
                    {
                        m_tileHeatMap->SetValueAtCoords(w, nextSearchValue);
                        isStillGoing = true;
                    }
                }
            }
        }
        currentSearchValue++;
    }

    printf("( Map%d ) Finish | GenerateDistanceField\n", m_mapDef->GetIndex());
}

//----------------------------------------------------------------------------------------------------
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
        case NUM_ENTITY_TYPES:
            ERROR_AND_DIE(Stringf("Unknown entity type #%i\n", type))
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------
void Map::AddEntityToMap(Entity* entity, Vec2 const& position, float const orientationDegrees)
{
    entity->m_map                = this;
    entity->m_position           = position;
    entity->m_orientationDegrees = orientationDegrees;

    AddEntityToList(entity, m_allEntities);
    AddEntityToList(entity, m_entitiesByType[entity->m_type]);

    if (IsBullet(entity))
        AddEntityToList(entity, m_bulletsByFaction[entity->m_faction]);

    if (IsAgent(entity))
        AddEntityToList(entity, m_agentsByFaction[entity->m_faction]);
}

//----------------------------------------------------------------------------------------------------
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
        RemoveEntityFromList(entity, m_agentsByFaction[entity->m_faction]);

    if (IsBullet(entity))
        RemoveEntityFromList(entity, m_bulletsByFaction[entity->m_faction]);

    entity->m_map = nullptr;
}

//----------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromList(Entity const* entity, EntityList& entityList)
{
    for (EntityList::iterator it = entityList.begin(); it != entityList.end(); ++it)
    {
        if (*it == entity)
        {
            entityList.erase(it);
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::DeleteGarbageEntities()
{
    for (Entity* entity : m_allEntities)
    {
        if (entity->m_isGarbage)
        {
            RemoveEntityFromMap(entity);
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::SpawnNewNPCs()
{
    printf("( Map%d ) Start  | SpawnNewNPCs\n", m_mapDef->GetIndex());

    for (int i = 0; i < m_dimensions.x * m_dimensions.y; ++i)
    {
        IntVec2 const randomTileCoords = RollRandomTileCoords();

        if (IsEdgeTile(randomTileCoords.x, randomTileCoords.y) ||
            IsTileSolid(IntVec2(randomTileCoords.x, randomTileCoords.y)) ||
            IsTileCoordsInLShape(randomTileCoords.x, randomTileCoords.y))
            continue;

        Vec2 const worldPosition(static_cast<float>(randomTileCoords.x) + 0.5f, static_cast<float>(randomTileCoords.y) + 0.5f);

        if (IsWorldPosOccupied(worldPosition))
            continue;

        switch (g_theRNG->RollRandomIntInRange(0, 3))
        {
            case 0:
                if (g_theRNG->RollRandomFloatZeroToOne() < m_mapDef->GetScorpioSpawnPercentage())
                    SpawnNewEntity(ENTITY_TYPE_SCORPIO, ENTITY_FACTION_EVIL, worldPosition, 0.f);

                break;

            case 1:
                if (g_theRNG->RollRandomFloatZeroToOne() < m_mapDef->GetLeoSpawnPercentage())
                    SpawnNewEntity(ENTITY_TYPE_LEO, ENTITY_FACTION_EVIL, worldPosition, 0.f);

                break;

            case 2:
                if (g_theRNG->RollRandomFloatZeroToOne() < m_mapDef->GetAriesSpawnPercentage())
                    SpawnNewEntity(ENTITY_TYPE_ARIES, ENTITY_FACTION_EVIL, worldPosition, 0.f);

                break;
        }
    }

    printf("( Map%d ) Finish | SpawnNewNPCs\n", m_mapDef->GetIndex());
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
        if (IsBullet(entity))
            continue;

        if (g_theGame->IsNoClip() && entity->m_type == ENTITY_TYPE_PLAYER_TANK)
            continue;

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
    if (!IsTileSolid(tileCoords))
        return;

    AABB2 const aabb2Box = GetTileBounds(tileCoords);

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

            bool const canAPushB = entityA->m_doesPushEntities && entityB->m_isPushedByEntities;
            bool const canBPushA = entityB->m_doesPushEntities && entityA->m_isPushedByEntities;

            if (canAPushB &&
                canBPushA)
            {
                PushDiscsOutOfEachOther2D(entityA->m_position,
                                          entityA->m_physicsRadius,
                                          entityB->m_position,
                                          entityB->m_physicsRadius);
            }

            if (!canAPushB &&
                canBPushA)
            {
                PushDiscOutOfDisc2D(entityA->m_position,
                                    entityA->m_physicsRadius,
                                    entityB->m_position,
                                    entityB->m_physicsRadius);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
void Map::CheckEntityVsEntityCollision(EntityList const& entityListA, EntityList const& entityListB)
{
    for (Entity* entityA : entityListA)
    {
        if (!entityA)
            continue;

        if (entityA->m_isDead)
            continue;

        for (Entity* entityB : entityListB)
        {
            if (!entityB)
                continue;

            if (entityB->m_isDead)
                continue;

            if (IsBullet(entityB))
                continue;

            if (entityA == entityB)
                continue;

            if (DoDiscsOverlap(entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius))
            {
                if (entityA->m_faction == entityB->m_faction)
                    continue;

                if (entityB->m_type == ENTITY_TYPE_ARIES)
                {
                    if (IsPointInsideDirectedSector2D(entityA->m_position, entityB->m_position, entityB->m_velocity.GetNormalized(), 90.f, entityB->m_physicsRadius * 1.5f))
                    {
                        RaycastResult2D const raycastResult2D   = RaycastVsDisc2D(entityA->m_position, entityA->m_velocity.GetNormalized(), entityA->m_velocity.GetLength(), entityB->m_position, entityB->m_physicsRadius);
                        Vec2 const            reflectedVelocity = entityA->m_velocity.GetReflected(raycastResult2D.m_impactNormal);

                        entityA->m_orientationDegrees = Atan2Degrees(reflectedVelocity.y, reflectedVelocity.x);
                        entityA->m_health--;
                        g_theAudio->StartSound(g_theGame->GetEnemyHitSoundID());
                        return;
                    }
                }

                entityA->m_health--;
                entityB->m_health--;

                if (entityB->m_type == ENTITY_TYPE_PLAYER_TANK)
                {
                    g_theAudio->StartSound(g_theGame->GetPlayerTankHitSoundID());
                }
                else
                {
                    g_theAudio->StartSound(g_theGame->GetEnemyHitSoundID());
                }
            }

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

        IntVec2 const tileCoords = GetTileCoordsFromWorldPos(currentPos);

        // Check bounds
        if (tileCoords.x < 0 || tileCoords.x >= m_dimensions.x ||
            tileCoords.y < 0 || tileCoords.y >= m_dimensions.y)
        {
            raycastResult.m_didImpact  = true;
            raycastResult.m_impactDist = t;
            raycastResult.m_impactPos  = currentPos;
            // TODO: FIX m_impactNormal logic ( nextPos - currentPos )
            // raycastResult.m_impactNormal =

            return raycastResult; // Out of bounds is considered blocking
        }

        // Check tile blocking
        int const tileIndex = tileCoords.y * m_dimensions.x + tileCoords.x;

        if (tileIndex >= 0 && tileIndex < static_cast<int>(m_tiles.size()))
        {
            Tile const& tile = m_tiles[tileIndex];

            if (tile.m_tileName == "Stone")
            {
                raycastResult.m_didImpact    = true;
                raycastResult.m_impactDist   = t;
                raycastResult.m_impactPos    = currentPos;
                AABB2 const tileBounds       = GetTileBounds(tileIndex);
                Vec2 const  nearestPoint     = tileBounds.GetNearestPoint(currentPos);
                raycastResult.m_impactNormal = (ray.m_origin - nearestPoint).GetNormalized();

                return raycastResult;
            }
        }
    }

    return raycastResult;
}
