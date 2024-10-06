// include other parts
#include "networking.hpp"
#include "configs.hpp"
#include <thread>

int main() {
    // initializes window and main components
    GameWindow window(1200, 780, 60, true);
    window.initialize();
    Map* gameMap = new Map(200);
    Controller* gameController = new Controller(gameMap);
    Sockets* sock = new Sockets("127.0.0.1", 128, 4277, gameController);
    std::thread network_thread;

    // loads game textures
    gameMap->loadTextures();

    // main game loop
    unsigned char player_number = 3;
    int gameState = 0;
    while(!WindowShouldClose()) {
        // menu loop
        if(gameState == 0) {
            gameState = window.menuDrawLoop(player_number) + 1;
            // start game
            if(gameState > 0 && gameState < 3) {
                gameController->clearController(player_number);
                sock->clearSocket();
                gameMap->initializeMap(player_number * 2);

                // begin game as server
                if(gameState == 1) {
                    if(sock->startServer() == -1) {
                        printf("Error starting server\n");
                        gameState = 0;
                    }
                    else {
                        network_thread = std::thread(&Sockets::realServerLoop, sock);
                    }
                }
                // begin game as client
                else {
                    if(sock->startClient() == -1) {
                        printf("Error starting client\n");
                        gameState = 0;
                    }
                    else {
                        network_thread = std::thread(&Sockets::realClientLoop, sock);
                    }
                }
            }
            // changes number of players
            if(gameState > 2) {
                player_number += (gameState * 2 - 7);
                gameState = 0;
            }
        }
        // game loop
        else {
            window.cameraUpdate();

            gameController->receiveMessages();

            // deals with mouse clicks
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                // checks region of the board where click occurs
                gameController->checkMapClicks(GetScreenToWorld2D(GetMousePosition(), window.camera));
            }

            // draws window (and returns to menu if "<" button is pressed)
            if(window.gameDrawLoop(gameController, gameMap) == 1) {
                sock->endConnection(&network_thread);
                gameState = 0;
            }

            // game execution needs to be halted
            else if(sock->halt_loop) {
                printf("Connection error, returning to menu\n");
                sock->endConnection(&network_thread);
                gameState = 0;
            }
        }
    }

    gameMap->unloadTextures();
    window.finalize();
    if(!sock->halt_loop && gameState != 0)
        sock->endConnection(&network_thread);

    // correct execution
    return 0;
}