// include other parts
#include "networking.hpp"
#include "game.hpp"
#include "configs.hpp"
#include <thread>

void ler(Sockets sock) {
    while(1) {
        std::string message;
        std::cin >> message;
        sock.sendMessageAll(message);
    }
}


int main() {
    // initializes game window
    //initialize();

    Sockets sock = Sockets("127.0.0.1", 128, 4277);

    int input;
    scanf("%d", &input);
    std::vector<std::string> messages;

    if(input == 1) {
        sock.startServer();
        while(1) {
            sock.loopServer(&messages);
            for(auto st : messages) {
                std::cout << st << std::endl;
                sock.closeSocket();
                return 0;
            }
        }
    }
    else {
        sock.startClient();
        std::string message;
        std::cin >> message;
        sock.sendMessage(message);
        sock.closeSocket();
    }

    // correct execution
    return 0;
}