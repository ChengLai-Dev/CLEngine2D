#include "TiledMap.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

TileMap::TileMap() = default;
TileMap::~TileMap() = default;

bool TileMap::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        Logger::Error("TileMap: failed to open '{}'", filepath);
        return false;
    }

    Clear();

    std::string line;
    int expectedWidth = 0;
    int expectedHeight = 0;
    int currentLayer = -1;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("tilesize:", 0) == 0) {
            std::istringstream iss(line.substr(9));
            iss >> m_tileWidth >> m_tileHeight;
            m_tileSet.tileWidth = m_tileWidth;
            m_tileSet.tileHeight = m_tileHeight;
        } else if (line.rfind("texture:", 0) == 0) {
            std::string texPath = line.substr(8);
            std::shared_ptr<Texture> tex = std::make_shared<Texture>(texPath);
            m_tileSet.texture = tex;
        } else if (line.rfind("texture_cols:", 0) == 0) {
            m_tileSet.columns = std::stoi(line.substr(13));
        } else if (line.rfind("texture_rows:", 0) == 0) {
            m_tileSet.rows = std::stoi(line.substr(13));
        } else if (line.rfind("map:", 0) == 0) {
            std::istringstream iss(line.substr(4));
            iss >> expectedWidth >> expectedHeight;
            m_width = expectedWidth;
            m_height = expectedHeight;

            TileMapLayer layer;
            layer.name = std::format("layer{}", m_layers.size());
            layer.width = expectedWidth;
            layer.height = expectedHeight;
            layer.tiles.resize(static_cast<size_t>(expectedWidth) * expectedHeight, -1);
            m_layers.push_back(layer);
            currentLayer = static_cast<int>(m_layers.size()) - 1;
        } else if (line.rfind("layer:", 0) == 0) {
            std::istringstream iss(line.substr(6));
            iss >> expectedWidth >> expectedHeight;

            std::string layerName;
            if (iss >> layerName) {
            }

            TileMapLayer layer;
            layer.name = layerName.empty() ? std::format("layer{}", m_layers.size()) : layerName;
            layer.width = expectedWidth;
            layer.height = expectedHeight;
            layer.tiles.resize(static_cast<size_t>(expectedWidth) * expectedHeight, -1);
            m_layers.push_back(layer);
            currentLayer = static_cast<int>(m_layers.size()) - 1;
        } else if (currentLayer >= 0 && currentLayer < static_cast<int>(m_layers.size())) {
            auto& layer = m_layers[currentLayer];
            std::istringstream iss(line);
            int tile;
            int col = 0;
            while (iss >> tile) {
                if (col < layer.width) {
                    int dataIdx = 0;
                    for (int y = 0; y < layer.height; ++y) {
                        bool found = false;
                        for (int x = 0; x < layer.width; ++x) {
                            if (layer.tiles[y * layer.width + x] == -1) {
                                dataIdx = y * layer.width + x;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    layer.tiles[dataIdx] = tile;
                }
                ++col;
            }
        }
    }

    Logger::Info("TileMap: loaded '{}' ({}x{}, {} tiles, {} layers)",
                             filepath, m_width, m_height,
                             m_tileSet.columns * m_tileSet.rows,
                             m_layers.size());
    return true;
}

bool TileMap::Create(int width, int height, int tileWidth, int tileHeight) {
    Clear();
    m_width = width;
    m_height = height;
    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;
    m_tileSet.tileWidth = tileWidth;
    m_tileSet.tileHeight = tileHeight;
    return true;
}

void TileMap::SetTileSet(const TileSet& tileset) {
    m_tileSet = tileset;
}

const TileSet& TileMap::GetTileSet() const {
    return m_tileSet;
}

int TileMap::AddLayer(const TileMapLayer& layer) {
    m_layers.push_back(layer);
    return static_cast<int>(m_layers.size()) - 1;
}

TileMapLayer* TileMap::GetLayer(int index) {
    if (index < 0 || index >= static_cast<int>(m_layers.size())) return nullptr;
    return &m_layers[index];
}

int TileMap::GetLayerCount() const {
    return static_cast<int>(m_layers.size());
}

void TileMap::SetTile(int layerIndex, int x, int y, int gid) {
    if (layerIndex < 0 || layerIndex >= static_cast<int>(m_layers.size())) return;
    auto& layer = m_layers[layerIndex];
    if (x < 0 || x >= layer.width || y < 0 || y >= layer.height) return;
    layer.tiles[y * layer.width + x] = gid;
}

int TileMap::GetTile(int layerIndex, int x, int y) const {
    if (layerIndex < 0 || layerIndex >= static_cast<int>(m_layers.size())) return -1;
    return m_layers[layerIndex].GetTile(x, y);
}

Vec2 TileMap::GetMapSize() const {
    return Vec2(static_cast<float>(m_width), static_cast<float>(m_height));
}

int TileMap::GetPixelWidth() const {
    return m_width * m_tileWidth;
}

int TileMap::GetPixelHeight() const {
    return m_height * m_tileHeight;
}

void TileMap::Render(Renderer& renderer, const Vec2& offset) const {
    if (!m_tileSet.texture) return;

    float texTileScaleX = static_cast<float>(m_tileSet.tileWidth) / static_cast<float>(m_tileSet.texture->GetWidth());
    float texTileScaleY = static_cast<float>(m_tileSet.tileHeight) / static_cast<float>(m_tileSet.texture->GetHeight());
    int tilesPerRow = m_tileSet.columns;

    float defaultColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    for (const auto& layer : m_layers) {
        if (!layer.visible) continue;

        Color layerOpacity(1.0f, 1.0f, 1.0f, layer.opacity);

        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                int gid = layer.tiles[y * layer.width + x];
                if (gid <= 0) continue;

                int tileIndex = gid - m_tileSet.firstGID;
                if (tileIndex < 0) continue;

                int tileCol = tileIndex % tilesPerRow;
                int tileRow = tileIndex / tilesPerRow;

                float texOffsetX = static_cast<float>(tileCol) * texTileScaleX;
                float texOffsetY = static_cast<float>(tileRow) * texTileScaleY;

                Vec3 position(
                    offset.x + static_cast<float>(x) * static_cast<float>(m_tileWidth) + static_cast<float>(m_tileWidth) * 0.5f,
                    offset.y + static_cast<float>(y) * static_cast<float>(m_tileHeight) + static_cast<float>(m_tileHeight) * 0.5f,
                    0.0f
                );

                renderer.DrawQuad(position,
                                  Vec3(static_cast<float>(m_tileWidth), static_cast<float>(m_tileHeight), 1.0f),
                                  0.0f,
                                  layerOpacity, m_tileSet.texture.get(),
                                  texOffsetX, texOffsetY,
                                  texTileScaleX, texTileScaleY);
            }
        }
    }
}

void TileMap::Clear() {
    m_layers.clear();
    m_tileSet = TileSet();
    m_width = 0;
    m_height = 0;
    m_tileWidth = 32;
    m_tileHeight = 32;
}
