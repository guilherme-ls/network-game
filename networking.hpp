#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

class Sockets {
    private:
        int connection_socket;

        const char *ip;

        const int STD_SIZE = 128;

        unsigned int connection_number;

        int port;

        struct timeval dropout_time = {0,0};

        std::vector<int> sockets_list;

    public:
        int startServer();

        int startClient();

        int loopClient(std::vector<std::string>* com);

        int loopServer(std::vector<std::string>* server_com);

        int sendMessage(std::string msg);

        int sendMessageAll(std::string msg);

        void closeSocket();

        Sockets(std::string ip, unsigned int connection_number, int port) {
            Sockets::ip = ip.c_str();
            Sockets::connection_number = connection_number;
            Sockets::port = port;
        }
};