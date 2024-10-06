#include <cstdlib>
#include <vector>
#include <array>
#include <mutex>
#include <sstream>
#include "map.hpp"

typedef struct {
    int i, j, k;
} vector3;

class Controller {
    public:
        int players;
        int player_turn = 0;
        int my_player = 0;
        Map* map;
        std::vector<bool> active_player;

        std::mutex mutex_alter_inbound_messages;
        std::mutex mutex_alter_outbound_messages;
        std::vector<std::string> inbound_messages;
        std::vector<std::string> outbound_messages;

        std::vector<std::array<std::array<int, 4>, 4>> positions;
        std::vector<std::array<std::array<bool, 4>, 4>> highlights;
        vector3 previous_piece = {10, 10, 10};

        void clearController(int players);

        void initialize_positions();

        void receiveMessages();
        
        void checkMapClicks(Vector2 mouse);

        void move(vector3 src, vector3 dst);

        void calculateMoves(vector3 src);

        Controller(Map* map) {
            Controller::map = map;
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