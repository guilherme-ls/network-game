#include "logic.hpp"


void Controller::initialize_positions() {
    for(int i = 0; i < Controller::players * 2; i++) {
        std::array<std::array<unsigned char, 4>, 4> temp;
        if(i % 2 == 1) {
            temp = {(std::array<unsigned char, 4>){0, 0, 6, 2}, (std::array<unsigned char, 4>){0, 0, 6, 4}, (std::array<unsigned char, 4>){0, 0, 6, 5}, (std::array<unsigned char, 4>){0, 0, 6, 3}};
            for(int j = 0; j < 4; j++) {
                for(int k = 2; k < 4; k++) {
                    temp[j][k] += 10 * (i - i % 2) / 2;
                }
            }
        }
        else {
            temp = {(std::array<unsigned char, 4>){0, 0, 0, 0}, (std::array<unsigned char, 4>){0, 0, 0, 0}, (std::array<unsigned char, 4>){6, 6, 6, 6}, (std::array<unsigned char, 4>){1, 4, 5, 3}};
            for(int j = 2; j < 4; j++) {
                for(int k = 0; k < 4; k++) {
                    temp[j][k] += 10 * (i - i % 2) / 2;
                }
            }
        }
        Controller::positions.emplace_back(temp);
    }
}

/**
 * @brief fills highlights matrix with false
 */
void Controller::fillFalseMatrix() {
    Controller::highlights.clear();
    std::array<std::array<bool, 4>, 4> false_matrix_fraction = {(std::array<bool, 4>){0, 0, 0, 0}, (std::array<bool, 4>){0, 0, 0, 0}, (std::array<bool, 4>){0, 0, 0, 0}, (std::array<bool, 4>){0, 0, 0, 0}};
    // fills everything with false
    for(int k = 0; k < Controller::players * 2; k++) {
        Controller::highlights.emplace_back(false_matrix_fraction);
    }
}

void Controller::move(vector3 src, vector3 dst) {
    Controller::positions[dst.i][dst.j][dst.k] = Controller::positions[src.i][src.j][src.k];
    Controller::positions[src.i][src.j][src.k] = 0;
}

void Controller::checkMapClicks(Vector2 mouse, Map map) {
    // loops over each fraction of the map
    for(int i = 0; i < map.fractions; i++) {
        // checks collision with each point
        for(int j = 0; j < 4; j++) {
            for(int k = 0; k < 4; k++) {
                bool collision = false;
                collision = CheckCollisionPointTriangle(mouse, map.edge_positions[i][j][k], map.edge_positions[i][j][k + 1], map.edge_positions[i][j + 1][k]);
                collision += CheckCollisionPointTriangle(mouse, map.edge_positions[i][j][k + 1], map.edge_positions[i][j + 1][k], map.edge_positions[i][j + 1][k + 1]);
                
                // deals with clicks
                if(collision) {
                    if(highlights[i][j][k]) {
                        fillFalseMatrix();
                        move(Controller::previous_piece, (vector3){i, j, k});
                        std::string msg = "mov " + std::to_string(previous_piece.i) + " " + std::to_string(previous_piece.j) + " " + std::to_string(previous_piece.k) + " " + std::to_string(i) + " " +std::to_string(j) + " " + std::to_string(k) + " ";
                        sendMessages(msg);
                        player_turn = (player_turn + 1) % players;
                    }
                    else if((Controller::positions[i][j][k] - Controller::positions[i][j][k] % 10) / 10 == player_turn) {
                        fillFalseMatrix();
                        previous_piece = {i, j, k};
                        calculateMoves((vector3){i, j, k});
                    }
                    return;
                }
            }
        }
    }
}

void Controller::calculateMoves(vector3 src) {
    switch(Controller::positions[src.i][src.j][src.k] % 10) {
        case 1:
            calculateKing(src);
            break;
        case 2:
            calculateQueen(src);
            break;
        case 3:
            calculateRook(src);
            break;
        case 4:
            calculateBishop(src);
            break;
        case 5:
            calculateHorse(src);
            break;
        case 6:
            calculatePawn(src);
            break;
        default:
            break;
    }
}

/**
 * @brief calculates direction of the "top"
 * @param i fraction of the board the piece is in
 * @param piece code of the piece
 * @return direction in a <x, y> pair
 */
std::pair<int, int> calculateDirection(int i, int piece) {
    if(i % 2 == 0) {
        if((i - i % 2) / 2 == (piece - piece % 10) / 10)
            return std::make_pair(-1, 0);
        else
            return std::make_pair(1, 0);
    }
    else {
        if((i - i % 2) / 2 == (piece - piece % 10) / 10)
            return std::make_pair(0, -1);
        else
            return std::make_pair(0, 1);
    }
}
// j = x, k = y.

bool Controller::checkBlock(int player, vector3 dest) {
    unsigned char piece = positions[dest.i][dest.j][dest.k];
    // checks if the way is blocked
    if(piece != 0) {
        // reaches piece of that can be eliminated
        if(player != (piece - piece % 10) / 10) {
            highlights[dest.i][dest.j][dest.k] = true;
        }
        return true;
    }
    return false;
}

void Controller::moveDirection(bool recursive, int player, std::pair<int, int> direction, vector3 src) {
    // updates position
    vector3 dest = {src.i, src.j + direction.first, src.k + direction.second};
    // checks if position is valid
    if(dest.j > 3 || dest.k > 3)
        return;
    // special case for diagonal moves
    if (src.j == 0 && src.k == 0 && dest.j == -1 && dest.k == -1) {
        direction = std::make_pair(1, 1);
        for(int pos = src.i % 2; pos < players * 2; pos += 2) {
            if(pos == src.i)
                continue;

            dest = {pos, 0, 0};
            if(checkBlock(player, dest))
                continue;

            if(positions[src.i][src.j][src.k] % 10 != 6)
                highlights[pos][0][0] = true;
            if(recursive)
                moveDirection(recursive, player, direction, dest);
        }

        return;
    }
    // horse movement
    if(dest.j < 0 && dest.k < 0) {
        dest = {(src.i + players * 2 - 2) % (players * 2), -dest.j - 1, -dest.k - 1};
        if(!checkBlock(player, dest))
            highlights[dest.i][dest.j][dest.k] = true;
        
        dest = {(src.i + 2) % (players * 2), dest.j, dest.k};
        if(!checkBlock(player, dest))
            highlights[dest.i][dest.j][dest.k] = true;
        
        return;
    }
    // general cases
    if(dest.j < 0) {
        dest = {(src.i + players * 2 - 1) % (players * 2), dest.k, -dest.j - 1};
        direction = std::make_pair(direction.second, -direction.first);
    }
    else if(dest.k < 0) {
        dest = {(src.i + 1) % (players * 2), -dest.k - 1, dest.j};
        direction = std::make_pair(-direction.second, direction.first);
    }
    
    // checks if another piece blocks the path
    if(checkBlock(player, dest)) {
        // overwrite collision if pawn
        if(positions[src.i][src.j][src.k] % 10 == 6 && abs(direction.first) != abs(direction.second))
            highlights[dest.i][dest.j][dest.k] = false;
        return;
    }
    
    // last update to position
    if(positions[src.i][src.j][src.k] % 10 != 6 || abs(direction.first) != abs(direction.second))
        highlights[dest.i][dest.j][dest.k] = true;
    // continues moving if recursive
    if(recursive) {
        moveDirection(recursive, player, direction, dest);
    }
}

void Controller::calculatePawn(vector3 src) {
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    std::pair<int, int> direction = calculateDirection(src.i, piece);
    std::pair<int, int> direction_double = direction;
    std::pair<int, int> direction_eat1 = direction;
    std::pair<int, int> direction_eat2 = direction;
    
    // normal movement
    moveDirection(false, player, direction, src);

    
    // double movement
    if(src.j == 2 && direction.first == -1) {
        direction_double.first = -2;
    }
    else if(src.k == 2 && direction.second == -1) {
        direction_double.second = -2;
    }
    moveDirection(false, player, direction_double, src);
    
    // eat piece movement
    if(direction.first != 0) {
        direction_eat1.second = 1;
        direction_eat2.second = -1;
    }
    if(direction.second != 0) {
        direction_eat1.first = 1;
        direction_eat2.first = -1;
    }
    moveDirection(false, player, direction_eat1, src);
    moveDirection(false, player, direction_eat2, src);
}

void Controller::calculateRook(vector3 src) {
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    moveDirection(true, player, std::make_pair(1, 0), src);
    moveDirection(true, player, std::make_pair(-1, 0), src);
    moveDirection(true, player, std::make_pair(0, 1), src);
    moveDirection(true, player, std::make_pair(0, -1), src);
}

void Controller::calculateHorse(vector3 src) {
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    moveDirection(false, player, std::make_pair(2, 1), src);
    moveDirection(false, player, std::make_pair(2, -1), src);
    moveDirection(false, player, std::make_pair(-2, 1), src);
    moveDirection(false, player, std::make_pair(-2, -1), src);
    moveDirection(false, player, std::make_pair(1, 2), src);
    moveDirection(false, player, std::make_pair(-1, 2), src);
    moveDirection(false, player, std::make_pair(1, -2), src);
    moveDirection(false, player, std::make_pair(-1, -2), src);
}

void Controller::calculateBishop(vector3 src) {
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    moveDirection(true, player, std::make_pair(1, 1), src);
    moveDirection(true, player, std::make_pair(-1, 1), src);
    moveDirection(true, player, std::make_pair(1, -1), src);
    moveDirection(true, player, std::make_pair(-1, -1), src);
}

void Controller::calculateQueen(vector3 src) {    
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    moveDirection(true, player, std::make_pair(1, 0), src);
    moveDirection(true, player, std::make_pair(-1, 0), src);
    moveDirection(true, player, std::make_pair(0, 1), src);
    moveDirection(true, player, std::make_pair(0, -1), src);
    moveDirection(true, player, std::make_pair(1, 1), src);
    moveDirection(true, player, std::make_pair(-1, 1), src);
    moveDirection(true, player, std::make_pair(1, -1), src);
    moveDirection(true, player, std::make_pair(-1, -1), src);
}

void Controller::calculateKing(vector3 src) {
    unsigned char piece = positions[src.i][src.j][src.k];
    int player = (piece - piece % 10) / 10;
    moveDirection(false, player, std::make_pair(1, 0), src);
    moveDirection(false, player, std::make_pair(-1, 0), src);
    moveDirection(false, player, std::make_pair(0, 1), src);
    moveDirection(false, player, std::make_pair(0, -1), src);
    moveDirection(false, player, std::make_pair(1, 1), src);
    moveDirection(false, player, std::make_pair(-1, 1), src);
    moveDirection(false, player, std::make_pair(1, -1), src);
    moveDirection(false, player, std::make_pair(-1, -1), src);
}


void Controller::receiveMessages() {
    // deals with received messages
    mutex_alter_inbound_messages.lock();
    int size = inbound_messages.size();
    mutex_alter_inbound_messages.unlock();

    for(int i = 0; i < size; i++) {
        std::string msg;
        mutex_alter_inbound_messages.lock();
        auto iter = inbound_messages.cbegin();
        msg = *iter;
        inbound_messages.erase(iter);
        mutex_alter_inbound_messages.unlock();
        
        // deal with msg
        executeMessage(msg);
    }
}

void Controller::executeMessage(std::string msg) {
    if(msg.substr(0,3) == "mov") {
        std::stringstream stream;
        stream << msg.substr(3, msg.size() - 3);
        vector3 src, dst;
        stream >> src.i >> src.j >> src.k >> dst.i >> dst.j >> dst.k;
        move(src, dst);
        player_turn = (player_turn + 1) % players;
    }
}

void Controller::sendMessages(std::string msg) {
    mutex_alter_outbound_messages.lock();
    outbound_messages.emplace_back(msg);
    mutex_alter_outbound_messages.unlock();
}