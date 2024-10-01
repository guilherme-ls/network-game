#include <cstdlib>
#include <vector>
#include <array>
#include <mutex>
#include <sstream>
#include "game.hpp"

typedef struct {
    int i, j, k;
} vector3;

class Controller {
    public:
        unsigned char players;
        unsigned char player_turn = 0;

        std::mutex mutex_alter_inbound_messages;
        std::mutex mutex_alter_outbound_messages;
        std::vector<std::string> inbound_messages;
        std::vector<std::string> outbound_messages;

        std::vector<std::array<std::array<unsigned char, 4>, 4>> positions;
        std::vector<std::array<std::array<bool, 4>, 4>> highlights;
        vector3 previous_piece = {10, 10, 10};

        void initialize_positions();

        void receiveMessages();
        
        void checkMapClicks(Vector2 mouse, Map map);

        void move(vector3 src, vector3 dst);

        void calculateMoves(vector3 src);

        Controller(unsigned char players) {
            Controller::players = players;
            fillFalseMatrix();
            initialize_positions();
        }

    private:
        void fillFalseMatrix();

        bool checkBlock(int player, vector3 dest);
        void moveDirection(bool recursive, int player, std::pair<int, int> direction, vector3 src);

        void calculatePawn(vector3 src);
        void calculateRook(vector3 src);
        void calculateHorse(vector3 src);
        void calculateBishop(vector3 src);
        void calculateQueen(vector3 src);
        void calculateKing(vector3 src);

        void executeMessage(std::string msg);
        void sendMessages(std::string msg);
};