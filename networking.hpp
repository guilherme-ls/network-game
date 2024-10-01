#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "logic.hpp"

class Sockets {
    private:
        int connection_socket;

        const char *ip;

        const int STD_SIZE = 128;

        unsigned int connection_number;

        int port;

        struct timeval dropout_time = {0,0};

        std::vector<int> sockets_list;
        
        Controller* control;

        std::mutex mutex_alter_socket_list;
        std::mutex mutex_alter_inbound_messages;
        std::mutex mutex_alter_outbound_messages;

        std::vector<std::pair<std::string, int>> inbound_messages;
        std::vector<std::pair<std::string, int>> outbound_messages;

        void receiveMessage(int nsock);

        void acceptConnections();

    public:
        int startServer();

        int startClient();

        int receiveLoopClient();

        int receiveLoopServer();

        void sendLoopClient();
        
        void sendLoopServer();

        int sendMessage(int fd, std::string msg);

        void closeSocket();

        void realClientLoop();

        void realServerLoop();

        Sockets(std::string ip, unsigned int connection_number, int port, Controller* control) {
            Sockets::ip = ip.c_str();
            Sockets::connection_number = connection_number;
            Sockets::port = port;
            Sockets::control = control;
        }
};
    // Sockets sock = Sockets("127.0.0.1", 128, 4277);