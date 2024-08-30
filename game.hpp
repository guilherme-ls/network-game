#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <string>
#include <vector>
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

        void drawMapTriangle(int fraction, int position, float scale);

        void finalize();

        void cameraUpdate();
};