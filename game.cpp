#include "game.hpp"

void GameWindow::initialize() {
    // sets antialiasing flag
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // initializes window with target resolution
    InitWindow(GameWindow::screenWidth, GameWindow::screenHeight, "Great Game");
    
    // sets target fps if option is selected
    if(GameWindow::fps_cap)
        SetTargetFPS(GameWindow::fps_target);
    
    // initializes camera
    GameWindow::camera = { 0 };
    GameWindow::camera.target = (Vector2){0, 0};
    GameWindow::camera.offset = (Vector2){600, 390};
    GameWindow::camera.rotation = 0.0f;
    GameWindow::camera.zoom = 1.0f;
}


void GameWindow::finalize() {
    CloseWindow();
}

/**
 * @brief draws background
 */
void GameWindow::generalDraw() {
    ClearBackground(RAYWHITE);
}

void GameWindow::drawMapTriangle(int fraction, int position, float scale) {
    // calculates angles
    float angle = 2 * PI / fraction;
    float starting = angle * position;

    // defines drawing constants
    const float line_thick = 3;
    const Color division_color = BLACK;

    // defines important points
    Vector2 points[5][5];
    points[0][0] = {0, 0}; // center
    points[0][4] = {0, scale}; // first slice point
    points[4][4] = {sin(angle/2)/cos(angle/2) * scale, scale}; // middle slice point
    points[4][0] = {sin(angle) * scale, cos(angle) * scale}; // last slice point

    // rotates main points
    points[0][4] = Vector2Rotate(points[0][4], -starting);
    points[4][4] = Vector2Rotate(points[4][4], -starting);
    points[4][0] = Vector2Rotate(points[4][0], -starting);

    // calculates square vertices and draw
    for(int i = 0; i < 5; i++) {
        points[i][4] = Vector2Add(Vector2Scale(points[0][4], 1 - 0.25 * i), Vector2Scale(points[4][4], 0.25 * i));
        points[i][0] = Vector2Add(Vector2Scale(points[0][0], 1 - 0.25 * i), Vector2Scale(points[4][0], 0.25 * i));
        for(int j = 1; j < 4; j++) {
            points[i][j] = Vector2Add(Vector2Scale(points[i][0], 1 - 0.25 * j), Vector2Scale(points[i][4], 0.25 * j));
        }
    }

    // draws squares
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            int color_number = (i + j + position) % 2;
            Color fill_color = {98 + 128 * color_number, 52 + 135 * color_number, 18 + 105 * color_number, 255};
            Vector2 drawn_points[4] = {points[i][j], points[i][j + 1], points[i + 1][j], points[i + 1][j + 1]};
            DrawTriangleStrip(drawn_points, 4, fill_color);
        }
    }
    
    // draws internal lines
    for(int i = 0; i < 5; i++) {
        DrawLineEx(points[i][0], points[i][4], line_thick, division_color);
        DrawLineEx(points[0][i], points[4][i], line_thick, division_color);
    }
}

void GameWindow::cameraUpdate() {
    GameWindow::camera.zoom += ((float)GetMouseWheelMove()*0.05f);

    if (IsKeyDown(KEY_RIGHT)) GameWindow::camera.target.x += 3;
    else if (IsKeyDown(KEY_LEFT)) GameWindow::camera.target.x -= 3;
    else if (IsKeyDown(KEY_UP)) GameWindow::camera.target.y -= 3;
    else if (IsKeyDown(KEY_DOWN)) GameWindow::camera.target.y += 3;

    if (GameWindow::camera.zoom > 10.0f) GameWindow::camera.zoom = 10.0f;
    else if (GameWindow::camera.zoom < 0.1f) GameWindow::camera.zoom = 0.1f;
}