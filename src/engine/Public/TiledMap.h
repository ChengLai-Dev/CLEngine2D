#pragma once

#include "Math/Vec2.h"

#include <memory>
#include <string>
#include <vector>

class Texture;
class Renderer;

struct TileSet {
    std::shared_ptr<Texture> texture = nullptr;
    int tileWidth = 32;
    int tileHeight = 32;
    int columns = 1;
    int rows = 1;
    int firstGID = 1;
};

struct TileMapLayer {
    std::string name;
    std::vector<int> tiles;
    int width = 0;
    int height = 0;
    bool visible = true;
    float opacity = 1.0f;

    int GetTile(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return -1;
        return tiles[y * width + x];
    }
};

class TileMap {
public:
    TileMap();
    ~TileMap();

    bool LoadFromFile(const std::string& filepath);
    bool Create(int width, int height, int tileWidth, int tileHeight);

    void SetTileSet(const TileSet& tileset);
    const TileSet& GetTileSet() const;

    int AddLayer(const TileMapLayer& layer);
    TileMapLayer* GetLayer(int index);
    int GetLayerCount() const;

    void SetTile(int layerIndex, int x, int y, int gid);
    int GetTile(int layerIndex, int x, int y) const;

    Vec2 GetMapSize() const;
    int GetPixelWidth() const;
    int GetPixelHeight() const;

    void Render(Renderer& renderer, const Vec2& offset = Vec2(0.0f, 0.0f)) const;

    void Clear();

private:
    TileSet m_tileSet;
    std::vector<TileMapLayer> m_layers;
    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 32;
    int m_tileHeight = 32;
};
