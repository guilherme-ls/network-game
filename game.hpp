#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include "logic.hpp"

class GameWindow {
    public:
        unsigned int screenWidth;
        unsigned int midScreenWidth;
        unsigned int screenHeight;
        unsigned int midScreenHeight;
        unsigned int fps_target;
        bool fps_cap;

        Font standard_font = LoadFont("resources/fonts/pixantiqua.png");

        Camera2D camera;

        GameWindow(unsigned int screenWidth, unsigned int screenHeight, unsigned int fps_target, bool fps_cap) {
            GameWindow::screenWidth = screenWidth;
            GameWindow::midScreenWidth = screenWidth/2;
            GameWindow::screenHeight = screenHeight;
            GameWindow::midScreenHeight= screenHeight/2;
            GameWindow::fps_target = fps_target;
            GameWindow::fps_cap = fps_cap;
        }

        int menuDrawLoop(int player_number, Map* decorativeMap);

        int gameDrawLoop(Controller* gameController, Map* gameMap);

        void initialize();

        void generalDraw();

        void finalize();

        void cameraUpdate();
    
    private:
        Rectangle drawButton(std::string text, Vector2 position, float scale, bool draw_rec);
};