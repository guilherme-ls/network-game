#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <iostream>

class Map {
    public:
        int fractions;
        std::vector<std::array<std::array<Vector2, 5>, 5>> edge_positions;
        std::vector<std::array<std::array<Vector2, 4>, 4>> piece_positions;
        std::vector<std::array<Texture2D, 6>> piece_textures;

        void drawMap();

        void drawPlayers(int highlighted_player);

        void calculatePoints(float scale);

        void calculatePiecePositions();

        void loadTextures();

        void unloadTextures();

        void drawPieces(std::vector<std::array<std::array<int, 4>, 4>> pieces, float piece_scale);

        void drawHighlights(std::vector<std::array<std::array<bool, 4>, 4>> highlight);

        void initializeMap(int fractions);

        Map(float scale) {
            Map::scale = scale;
        }

    private:
        float scale;
};