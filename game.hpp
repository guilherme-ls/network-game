#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <iostream>

class GameWindow {
    public:
        unsigned int screenWidth;
        unsigned int midScreenWidth;
        unsigned int screenHeight;
        unsigned int midScreenHeight;
        unsigned int fps_target;
        bool fps_cap;

        Camera2D camera;

        GameWindow(unsigned int screenWidth, unsigned int screenHeight, unsigned int fps_target, bool fps_cap) {
            GameWindow::screenWidth = screenWidth;
            GameWindow::midScreenWidth = screenWidth/2;
            GameWindow::screenHeight = screenHeight;
            GameWindow::midScreenHeight= screenHeight/2;
            GameWindow::fps_target = fps_target;
            GameWindow::fps_cap = fps_cap;
        }

        void initialize();

        void generalDraw();

        void finalize();

        void cameraUpdate();
};

class Map {
    public:
        int fractions;
        std::vector<std::array<std::array<Vector2, 5>, 5>> edge_positions;
        std::vector<std::array<std::array<Vector2, 4>, 4>> piece_positions;
        std::vector<std::array<Texture2D, 6>> piece_textures;

        void drawMenu();

        void drawMap();

        void calculatePoints(float scale);

        void calculatePiecePositions();

        void loadTextures();

        void unloadTextures();

        void drawPieces(std::vector<std::array<std::array<unsigned char, 4>, 4>> pieces, float piece_scale);

        void drawHighlights(std::vector<std::array<std::array<bool, 4>, 4>> highlight);

        Map(int fractions, float scale) {
            Map::fractions = fractions;
            calculatePoints(scale);
            calculatePiecePositions();
        }
};