#include "map.hpp"

/**
 * @brief Calculates all relevant points of the map
 * @param fractions number of fractions in the map
 */
void Map::initializeMap(int fractions) {
    Map::fractions = fractions;
    edge_positions.clear();
    piece_positions.clear();
    calculatePoints(scale);
    calculatePiecePositions();
}

/**
 * @brief calculates all relevant points in the map
 * @param scale scale for the map to be calculated (important when playing with >3 people)
 */
void Map::calculatePoints(float scale) {
    // calculates angles
    float angle = 2 * PI / fractions;

    // calculates points for each triangle part
    for(int position = 0; position < fractions; position++) {
        float starting = angle * position;

        // defines important points
        std::array<std::array<Vector2, 5>, 5> points;
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
        Map::edge_positions.emplace_back(points);
    }

    // calculates points for each triangle part
    for(int position = 0; position < fractions; position++) {
        float starting = angle * position;

        // defines important points
        std::array<std::array<Vector2, 5>, 5> points;
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
        Map::edge_positions.emplace_back(points);
    }
}

/**
 * @brief calculates coordinates for the pieces to be placed in the board
 */
void Map::calculatePiecePositions() {
    std::array<std::array<Vector2, 4>, 4> points;

    for(int i = 0; i < Map::fractions; i++) {
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                points[j][k] = Vector2Negate(Vector2Add(Map::edge_positions[i][j][k], Vector2Scale(Vector2Subtract(Map::edge_positions[i][j+1][k+1], Map::edge_positions[i][j][k]), 0.5)));
            }
        }
        Map::piece_positions.emplace_back(points);
    }
}

/**
 * @brief draws entire chess map
 */
void Map::drawMap() {
    // defines drawing constants
    const float line_thick = 3;
    const Color division_color = BLACK;

    // loops over each fraction of the map
    for(int k = 0; k < Map::fractions; k++) {
        // draws squares
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                int color_number = (i + j + k) % 2;
                Color fill_color = {(unsigned char)(98 + 128 * color_number), (unsigned char)(52 + 135 * color_number), (unsigned char)(18 + 105 * color_number), (int)255};
                Vector2 drawn_points[4] = {Map::edge_positions[k][i][j], Map::edge_positions[k][i][j + 1], Map::edge_positions[k][i + 1][j], Map::edge_positions[k][i + 1][j + 1]};
                DrawTriangleStrip(drawn_points, 4, fill_color);
            }
        }

        // draws internal lines
        for(int i = 0; i < 5; i++) {
            DrawLineEx(Map::edge_positions[k][i][0], Map::edge_positions[k][i][4], line_thick, division_color);
            DrawLineEx(Map::edge_positions[k][0][i], Map::edge_positions[k][4][i], line_thick, division_color);
        }
    }
}

/**
 * @brief draws player numbers arrounf the board
 * @param highlighted_player player to be highlighted in the current turn
 */
void Map::drawPlayers(int highlighted_player) {
    for(int i = 0; i < fractions / 2; i++) {
        Vector2 pos = Vector2Scale(edge_positions[i * 2][4][0], 1.3f);
        if(i == highlighted_player)
            DrawCircleV(pos, scale / 7.0f, RED);
        DrawCircleV(pos, scale / 8.0f, BLACK);
        std::string text = std::to_string(i + 1);
        Vector2 offset = MeasureTextEx(GetFontDefault(), text.c_str(), scale / 5.0f, scale / 40.0f);
        pos = Vector2Subtract(pos, Vector2Scale(offset, 0.5));
        DrawTextEx(GetFontDefault(), text.c_str(), pos, scale / 5.0f, scale / 40.0f, WHITE);
    }
}

/**
 * @brief loads piece textures into memory
 */
void Map::loadTextures() {
    std::array<Texture2D, 6> texture_set;
    char file[21];
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 6; j++) {
            sprintf(file, "Sprite/piece_%d_%d.png", j, i);
            texture_set[j] = LoadTexture(file);
        }
        Map::piece_textures.emplace_back(texture_set);
    }
}

/**
 * @brief unloads textures from memory
 */
void Map::unloadTextures() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 6; j++) {
            UnloadTexture(Map::piece_textures[i][j]);
        }
    }
}

/**
 * @brief draws chess pieces
 * @param pieces matrix of pieces in the board
 */
void Map::drawPieces(std::vector<std::array<std::array<int, 4>, 4>> pieces, float piece_scale) {
    for(int i = 0; i < Map::fractions; i++) {
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                if(pieces[i][j][k] != 0) {
                    int player = (pieces[i][j][k] - pieces[i][j][k] % 10) / 10;
                    int piece = pieces[i][j][k] % 10 - 1;
                    Rectangle original = {0.0, 0.0, (float)Map::piece_textures[0][piece].width, (float)Map::piece_textures[0][piece].height};
                    float sides = 100 * piece_scale;
                    Rectangle scale = {-sides/2, -sides/2, sides, sides};
                    Vector2 destination = Map::piece_positions[i][j][k];

                    if(player == Map::fractions/2 - 1) {
                        DrawTexturePro(Map::piece_textures[1][piece], original, scale, destination, 0.0, WHITE);
                    }
                    else {
                        unsigned char mult = (unsigned char)(255.0 * (1.0 - player/(Map::fractions/2 - 1.0)));
                        Color piece_color = {mult, mult, mult, 255};
                        DrawTexturePro(Map::piece_textures[0][piece], original, scale, destination, 0.0, piece_color);
                    }
                }
            }
        }
    }
}

void Map::drawHighlights(std::vector<std::array<std::array<bool, 4>, 4>> highlight) {
    // loops over each fraction of the map
    for(int i = 0; i < Map::fractions; i++) {
        // checks highlight in each point
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                if(highlight[i][j][k]) {
                    Vector2 drawn_points[4] = {Map::edge_positions[i][j][k], Map::edge_positions[i][j][k + 1], Map::edge_positions[i][j + 1][k], Map::edge_positions[i][j + 1][k + 1]};
                    DrawTriangleStrip(drawn_points, 4, RED);
                }
            }
        }
    }
}