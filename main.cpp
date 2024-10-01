// include other parts
#include "networking.hpp"
#include "configs.hpp"
#include <thread>

int main() {
    // initializes everything
    const int players = 3;
    GameWindow window(1200, 780, 60, true);
    window.initialize();
    Map gameMap(players * 2, 200);
    Controller gameController(players);
    Sockets sock("127.0.0.1", 128, 4277, &gameController);

    int input;
    std::cin >> input;
    // starts network thread
    std::thread network_thread;
    if(input == 1) {
        sock.startServer();
        network_thread = std::thread(&Sockets::realServerLoop, &sock);
    }
    else {
        sock.startClient();
        network_thread = std::thread(&Sockets::realClientLoop, &sock);
    }

    gameMap.loadTextures();

    while(!WindowShouldClose()) {
        window.cameraUpdate();

        gameController.receiveMessages();

        // starts drawing call
        BeginDrawing();
        window.generalDraw();
        
        // begins part affected by camera
        BeginMode2D(window.camera);

            // draws game map
            gameMap.drawMap();
            
            // deals with mouse clicks
            if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                // checks region of the board where click occurs
                gameController.checkMapClicks(GetScreenToWorld2D(GetMousePosition(), window.camera), gameMap);
            }

            // draws highlighted areas of the board
            gameMap.drawHighlights(gameController.highlights);

            // dras pieces of the board
            gameMap.drawPieces(gameController.positions, 1.0 / players);

        // end part affected by camera
        EndMode2D();

        // ends drawing call
        EndDrawing();
    }

    gameMap.unloadTextures();
    window.finalize();
    network_thread.~thread();
    sock.closeSocket();

    // correct execution
    return 0;
}